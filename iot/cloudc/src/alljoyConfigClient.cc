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

#include <signal.h>
#include <set>

#include <alljoyn/config/ConfigClient.h>
#include <alljoyn/AboutData.h>
#include <alljoyn/AboutListener.h>
#include <alljoyn/AboutObjectDescription.h>
#include <alljoyn/AboutProxy.h>
#include <alljoyn/AboutIconProxy.h>
#include <alljoyn/version.h>
#include <alljoyn/services_common/LogModulesNames.h>
#include "samples_common/SrpKeyXListener.h"
#include "samples_common/AsyncSessionJoiner.h"
#include "samples_common/SessionListenerImpl.h"
#include <iostream>
#include <iomanip>
#include <string.h>
#include "NewConfigClient.h"

/* notification add by gaojing start 2016.06.28 */
#include <sstream>
#include <alljoyn/PasswordManager.h>
#include <alljoyn/notification/NotificationService.h>
#include "common/NotificationReceiverTestImpl.h"
/* notification add by gaojing end 2016.06.28 */

extern "C"
{
#include "parser.h"
#include "share.h"
int cloudc_send_online_buf(char *device_id, char *device_type, struct ipk_info *about_data_head);
void cloudc_get_ipk_name(struct ipk_info *list_head, char *ipk_name, char *node_value);
void cloudc_print_ipk_name_list(struct ipk_info *list_head);
void cloudc_destroy_ipk_name(struct ipk_info *list_head);
void getDevInfoByObj(char *interfaceName, char *objectPath, char *deviceId, char *deviceType);
}

extern "C" void StartAlljoynService();
extern "C" void StopAlljoynService();
extern "C" int configClientMain(char *interfaces, char *matchObjectPath, int msgType, struct ipk_info *configInfoHead, int keyNameNumber);
void HandleUpdateConfigMethod(const char *interfaces);
void UpdateConfig(qcc::String const& busName, unsigned int id, const char *interfaceName, const char *objectPath, int msgType, struct ipk_info *configInfoHead, int keyNameNumber);
int CheckIfConfigInterfaceOnline(ajn::MsgArg objectDescriptionArg, char *interfaceName, char *objectPath);
int ParseGetAboutDataArg(ajn::MsgArg aboutDataArg, struct ipk_info *listHead);
int InteresteForAllConfigInterfaces(const char **interfaces);
int NotificationInit();

unsigned int remoteConfigFlag = 0;
unsigned int callbackFinish = 0;
char* busname = NULL;
char* objectPath = NULL;
char* interfaceName = NULL;
int getMsgType = -1;
int getKeyNameNumber = -1;
struct ipk_info *getConfigInfoHead = NULL;
typedef enum 
{
    Get,
    Set,
    Notify
}MsgType;

using namespace ajn;
using namespace services;

#define INITIAL_PASSCODE "000000"
#define NEW_PASSCODE "12345678"

static BusAttachment* busAttachment = 0;
static SrpKeyXListener* srpKeyXListener = 0;
static std::set<qcc::String> handledAnnouncements;

static volatile sig_atomic_t s_interrupt = false;
static volatile sig_atomic_t s_stopped = false;

static void SigIntHandler(int sig) {
    s_interrupt = true;
}

/* notification add by gaojing start */
NotificationService* conService = 0;
NotificationReceiverTestImpl* Receiver = 0;
static volatile sig_atomic_t s_interrupt_notify = false;

void cleanup()
{
    std::cout << "cleanup() - start" << std::endl;
    if (conService) {
        std::cout << "cleanup() - conService" << std::endl;
        conService->shutdown();
        conService = NULL;
    }   
    if (Receiver) {
        std::cout << "cleanup() - Receiver" << std::endl;
        delete Receiver;
        Receiver = NULL;
    }
#if 0
    if (busAttachment) {
        std::cout << "cleanup() - busAttachment" << std::endl;
        delete busAttachment;
        busAttachment = NULL;
    }
#endif
    std::cout << "cleanup() - end" << std::endl;
}
/* notification add by gaojing end */

/* Print out the fields found in the AboutData. Only fields with known signatures are printed out. 
 * All others will be treated as an unknown field.
 */
void printAboutData(AboutData& aboutData, const char* language)
{
    size_t count = aboutData.GetFields();

    const char** fields = new const char*[count];
    aboutData.GetFields(fields, count);

    for (size_t i = 0; i < count; ++i) {
        std::cout << "\tKey: " << fields[i];

        MsgArg* tmp;
        aboutData.GetField(fields[i], tmp, language);
        std::cout << "\t";
        if (tmp->Signature() == "s") {
            const char* tmp_s;
            tmp->Get("s", &tmp_s);
            std::cout << tmp_s;
        } else if (tmp->Signature() == "as") {
            size_t las;
            MsgArg* as_arg;
            tmp->Get("as", &las, &as_arg);
            for (size_t j = 0; j < las; ++j) {
                const char* tmp_s;
                as_arg[j].Get("s", &tmp_s);
                std::cout << tmp_s << " ";
            }
        } else if (tmp->Signature() == "ay") {
            size_t lay;
            uint8_t* pay;
            tmp->Get("ay", &lay, &pay);
            for (size_t j = 0; j < lay; ++j) {
                std::cout << std::hex << static_cast<int>(pay[j]) << " ";
            }
        } else {
            std::cout << "User Defined Value\tSignature: " << tmp->Signature().c_str();
        }
        std::cout << std::endl;
    }
    delete [] fields;
    std::cout << std::endl;
}

void printAllAboutData(AboutProxy& aboutProxy)
{
    MsgArg aArg;
    QStatus status = aboutProxy.GetAboutData(NULL, aArg);
    if (status == ER_OK) {
        std::cout << "*********************************************************************************" << std::endl;
        std::cout << "GetAboutData: (Default Language)" << std::endl;
        AboutData aboutData(aArg);
        printAboutData(aboutData, NULL);
        size_t lang_num;
        lang_num = aboutData.GetSupportedLanguages();
        std::cout << "Number of supported languages: " << lang_num << std::endl;
        // If the lang_num == 1 we only have a default language
        if (lang_num > 1) {
            const char** langs = new const char*[lang_num];
            aboutData.GetSupportedLanguages(langs, lang_num);
            char* defaultLanguage;
            aboutData.GetDefaultLanguage(&defaultLanguage);
            // print out the AboutData for every language but the
            // default it has already been printed.
            for (size_t i = 0; i < lang_num; ++i) {
                std::cout << "language=" << i << " " << langs[i] << std::endl;
                if (strcmp(defaultLanguage, langs[i]) != 0) {
                    std::cout << "Calling GetAboutData: language=" << langs[i] << std::endl;
                    status = aboutProxy.GetAboutData(langs[i], aArg);
                    if (ER_OK == status) {
                        aboutData.CreatefromMsgArg(aArg, langs[i]);
                        std::cout <<  "GetAboutData: (" << langs[i] << ")" << std::endl;
                        printAboutData(aboutData, langs[i]);
                    } else {
                        std::cout <<  "GetAboutData failed " << QCC_StatusText(status)  << std::endl;
                    }
                }
            }
            delete [] langs;
        }
        std::cout << "*********************************************************************************" << std::endl;
    }
}

void interruptibleDelay(int seconds) {
    for (int i = 0; !s_interrupt && i < seconds; i++) {
#ifdef _WIN32
        Sleep(1000);
#else
        usleep(1000 * 1000);
#endif
    }
}

class MyAboutListener : public AboutListener {
    void Announced(const char* busName, uint16_t version, SessionPort port, const MsgArg& objectDescriptionArg, const MsgArg& aboutDataArg)
    {
        std::cout<<"GaoJing Enter Announced func"<<std::endl;
        std::cout<<"GaoJing busname = "<<busName << ", version = " << version << ", port = " << port <<""<<std::endl;

        QStatus status = ER_OK;
        /* check if the device is 1st online */
        std::set<qcc::String>::iterator searchIterator = handledAnnouncements.find(qcc::String(busName));
        if (searchIterator == handledAnnouncements.end()) 
        {

            /* need to add func: send online info to server 
             * if enter this switch, it means this is the device 1st online
             * so just find deviceId by objectPath and interfaceName in objectDescriptionArg
             * then send it to server 
             * */
            char thisInterfaceName[MAX_INTERFACE_LEN] = {0};
            char thisObjectPath[MAX_OBJECTPATH_LEN] = {0};
            char deviceId[MAX_DEVICE_ID_LEN] = {0};
            char deviceType[MAX_DEVICE_TYPE_LEN] = {0};
            struct ipk_info *aboutDataValue = 0;

            handledAnnouncements.insert(busName);
            CheckIfConfigInterfaceOnline(objectDescriptionArg, thisInterfaceName, thisObjectPath);
            getDevInfoByObj(thisInterfaceName, thisObjectPath, deviceId, deviceType);
            aboutDataValue = (struct ipk_info *)malloc(sizeof(struct ipk_info));

            if (NULL == aboutDataValue)
            {   
                printf("malloc failed\n");
            }   

            memset(aboutDataValue, 0, sizeof(struct ipk_info));
            ParseGetAboutDataArg(aboutDataArg, aboutDataValue);
            //cloudc_print_ipk_name_list(aboutDataValue->next);
            cloudc_send_online_buf(deviceId, deviceType, aboutDataValue->next); //need to add device_id as arg here
            cloudc_destroy_ipk_name(aboutDataValue);
            aboutDataValue = 0;
            cloudc_print_ipk_name_list(aboutDataValue);

            sleep(3);
            std::cout << "1111111111111111111111111111" << std::endl;
        }

        if(remoteConfigFlag == 1)
        {
            busAttachment->EnableConcurrentCallbacks();
#if 0
            // when pinging a remote bus wait a max of 5 seconds

#define PING_WAIT_TIME    5000
            QStatus status = busAttachment->Ping(busName, 5000);
            if( ER_OK == status) 
            {
                std::cout << "GaoJing ping succeed" << std::endl;
            }
            else
            {
                std::cout << "GaoJing ping fail" << std::endl;
            }
#endif

            SessionId sessionId;
            SessionOpts opts(SessionOpts::TRAFFIC_MESSAGES, false,
                    SessionOpts::PROXIMITY_ANY, TRANSPORT_ANY);
            status = busAttachment->JoinSession(busName, port, NULL, sessionId, opts);

            if(status == ER_OK)
            {
                std::cout << "JoinSession Success, sessionId = "<<sessionId<<"" << std::endl;
                remoteConfigFlag = 0;
                std::cout << "objectPath = " << objectPath << std::endl;
                std::cout << "interfaceName = " << interfaceName << std::endl;
                UpdateConfig(busName, sessionId, interfaceName, objectPath, getMsgType, getConfigInfoHead, getKeyNameNumber);
            }
            else
            {
                std::cout << "WARNING - JoinSession failed: " << QCC_StatusText(status) << std::endl;
            }
        }
        else
        {
            std::cout << "GaoJing This announce is no use" <<std::endl;
        }


        std::cout<<"GaoJing Exit Announced func"<<std::endl;
    }
};

static MyAboutListener* aboutListener = 0;
void WaitForSigInt(void) {
    std::cout<<"GaoJing Enter wait sigInt func "<<std::endl;
    while (s_interrupt == false && s_stopped == false) {
#ifdef _WIN32
        Sleep(100);
#else
        usleep(100 * 1000);
#endif
    }
    std::cout<<"GaoJing Exit wait sigInt func "<<std::endl;
}

int CreateBusAttachment(void) 
{
    QStatus status = ER_OK;
    std::cout << "AllJoyn Library version: " << ajn::GetVersion() << std::endl;
    std::cout << "AllJoyn Library build info: " << ajn::GetBuildInfo() << std::endl;

    std::cout << "*********************************************************************************" << std::endl;
    std::cout << "PLEASE NOTE THAT AS OF NOW THIS PROGRAM DOES NOT SUPPORT INTERACTION WITH THE ALLJOYN THIN CLIENT BASED CONFIGSAMPLE. SO PLEASE USE THIS PROGRAM ONLY WITH ALLJOYN STANDARD CLIENT BASED CONFIGSERVICESAMPLE" << std::endl;
    std::cout << "*********************************************************************************" << std::endl;

    //Enable this line to see logs from config service:
    //QCC_SetDebugLevel(services::logModules::CONFIG_MODULE_LOG_NAME, services::logModules::ALL_LOG_LEVELS);

    /* Install SIGINT handler so Ctrl + C deallocates memory properly */

    signal(SIGINT, SigIntHandler);

    //set Daemon password only for bundled app
#ifdef QCC_USING_BD
    PasswordManager::SetCredentials("ALLJOYN_PIN_KEYX", "000000");
#endif

    busAttachment = new BusAttachment("CloudcConfig", true);

    status = busAttachment->Start();
    if (status == ER_OK) {
        std::cout << "BusAttachment started." << std::endl;
    } else {
        std::cout << "ERROR - Unable to start BusAttachment. Status: " << QCC_StatusText(status) << std::endl;
        return 1;
    }

    status = busAttachment->Connect();
    if (ER_OK == status) {
        std::cout << "Daemon Connect succeeded." << std::endl;
    } else {
        std::cout << "ERROR - Failed to connect daemon. Status: " << QCC_StatusText(status) << std::endl;
        return 1;
    }
    return 0;
}

int CheckIfConfigInterfaceOnline(ajn::MsgArg objectDescriptionArg, char *interfaceName, char *objectPath)
{
    QStatus status = ER_OK;
    char *deviceObjectPath = NULL;
    char *deviceInterfaceName = NULL;
    MsgArg* tempArgs;
    size_t objectNum;
    status = objectDescriptionArg.Get("a(oas)", &objectNum, &tempArgs);
    std::cout << "GaoJing objectNum = " << objectNum << std::endl;

    if(status != ER_OK)
    {
        return -1;
    }

    for(size_t i = 0; i < objectNum; i ++)
    {
        char* objectDescriptionPath;
        MsgArg* interfaceEnteries;
        size_t interfaceNum;

        status = tempArgs[i].Get("(oas)", &objectDescriptionPath, &interfaceNum, &interfaceEnteries);
        std::cout << "GaoJing objectDescriptionPath " << i << ": " << objectDescriptionPath << ", its interfaceNum = " << interfaceNum << std::endl;

        if(strstr(objectDescriptionPath, "/Config") == NULL)
        {
            std::cout << "Not found ConfigInterface on this objectPath" << std::endl;
            continue;
        }

        deviceObjectPath = objectDescriptionPath;
        strcpy(objectPath, deviceObjectPath);

        if(status != ER_OK)
        {
            return -1;
        }

        for(size_t i = 0; i < interfaceNum; i ++)
        {
            char* interfaceOfAnnounce;
            status = interfaceEnteries[i].Get("s", &interfaceOfAnnounce);
            std::cout << "GaoJing InterfaceOfAnnounce " << i << ": " << interfaceOfAnnounce << std::endl;
            if(status != ER_OK)
            {
                return -1;
            }
            deviceInterfaceName = interfaceOfAnnounce;
            strcpy(interfaceName, deviceInterfaceName);
#if 0
            if(strstr(interfaceOfAnnounce, ".Config"))
            {
                deviceInterfaceName = interfaceOfAnnounce;
                break;
            }
#endif
        }

        std::cout << "deviceObjectPath = " << deviceObjectPath << ", deviceInterfaceName = " << deviceInterfaceName << std::endl;
        /* Need to add a func to check whether this configInterface is online or not.
         * if already online on the dataLib, do noting
         * if not online on the dataLib, (and this is its first time to online,) then need to send this info to FEIXUN server.
         * */
    }

    return 0;
}

int ParseGetAboutDataArg(ajn::MsgArg aboutDataArg, struct ipk_info *listHead)
{
    std::cout << "GaoJing Enter ParseGetAboutDataArg func" << std::endl;
    QStatus status = ER_OK;
    AboutData aboutData;
    MsgArg* tempControlArg2;
    size_t languageTagNumElements;

    status = aboutDataArg.Get("a{sv}", &languageTagNumElements, &tempControlArg2);
    if (status != ER_OK) 
    {
        std::cout << "Exit ParseGetAboutDataArg func" << std::endl;
        return -1;
    }

    for (size_t i = 0; i < languageTagNumElements; i++) 
    {
        char* tempKey;
        MsgArg* tempValue;
        status = tempControlArg2[i].Get("{sv}", &tempKey, &tempValue);
        if (status != ER_OK) {
            std::cout << "Exit ParseGetAboutDataArg func" << std::endl;
            return -1;
        }
        std::cout << "tempKey = " << tempKey << std::endl;
        std::cout << "tempValue = " << tempValue << std::endl;
        char* value = NULL;
        status = tempValue->Get("s", &value);
        if (status == ER_OK) {
            printf("value = %s\n", value);
            cloudc_get_ipk_name(listHead, tempKey, value);
        }
        else
        {
            printf("cannot get the value\n");
        }
        //aboutData.insert(std::pair<qcc::String, ajn::MsgArg>(tempKey, *tempValue));
    }

    std::cout << "GaoJing Exit ParseGetAboutDataArg func" << std::endl;
    return 0;
}

void HandleUpdateConfigMethod(const char *interfaces)
{
    std::cout<<"GaoJing Enter HandleUpdateConfigMethod func"<<std::endl;
    QStatus status = ER_OK;
    busAttachment->CancelWhoImplements(interfaces);
    status = busAttachment->WhoImplements(interfaces);
    if (ER_OK == status) {
        std::cout << "WhoImplements called." << std::endl;
    } else {
        std::cout << "ERROR - WhoImplements call FAILED with status " << QCC_StatusText(status) << std::endl;
    }

    std::cout<<"GaoJing Exit HandleUpdateConfigMethod func"<<std::endl;
    return ;
}

void UpdateConfig(qcc::String const& busName, unsigned int id, const char *interfaceName, const char *objectPath, int msgType, struct ipk_info *configInfoHead, int keyNameNumber)
{
    MsgType msgTypeOption;
    QStatus status = ER_OK;
    struct ipk_info *tmpConfigInfoHead = configInfoHead;

    std::cout<<"GaoJing Enter UpdateConfig func"<<std::endl;
    AboutProxy aboutProxy(*busAttachment, busName.c_str(), id);

    MsgArg objArg;
    aboutProxy.GetObjectDescription(objArg);
    std::cout << "AboutProxy.GetObjectDescriptions:\n" << objArg.ToString().c_str() << "\n\n" << std::endl;

    AboutObjectDescription objectDescription;
    objectDescription.CreateFromMsgArg(objArg);

    bool isConfigInterface = false;
    if (!s_interrupt) {
        isConfigInterface = objectDescription.HasInterface(objectPath, interfaceName);
        if (isConfigInterface) {
            std::cout << "The given interface '" << interfaceName << "' is found in a given path ' " << objectPath << std::endl;
        } else {
            std::cout << "WARNING - The given interface '" << interfaceName << "' is not found in a given path ' " << objectPath << std::endl;
        }
        //printAllAboutData(aboutProxy);
    }

    services::NewConfigClient* newConfigClient = NULL;
    if (!s_interrupt && isConfigInterface) {
        newConfigClient = new services::NewConfigClient(interfaceName, *busAttachment);
        if (!s_interrupt && newConfigClient) {

            switch(msgType)
            {
                case Get:
                    std::cout << "Get" << std::endl;

                    for(int i = 0; i < keyNameNumber ; i ++)
                    {
                        if(tmpConfigInfoHead->node_name_convert == NULL)
                        {
                            std::cout << "The keyName you want to get is NULL, please recheck" << std::endl;;
                            continue;
                        }
                        else
                        {
                            if (!s_interrupt) {
                                services::NewConfigClient::Configurations configurations;
                                std::cout << "\nNewConfigClient GetConfigurations (en), Get key name <" << tmpConfigInfoHead->node_name_convert << "> value" << std::endl;
                                std::cout << "-----------------------------------" << std::endl;

                                if ((status = newConfigClient->GetConfigurations(busName.c_str(), "en", configurations, objectPath, interfaceName, id))
                                        == ER_OK) {

                                    for (services::NewConfigClient::Configurations::iterator it = configurations.begin();
                                            it != configurations.end(); ++it) {
                                        qcc::String key = it->first;
                                        ajn::MsgArg value = it->second;
                                        if (value.typeId == ALLJOYN_STRING) {
                                            if(strcmp(key.c_str(), tmpConfigInfoHead->node_name_convert) == 0)
                                            {
                                                std::cout << "GaoJing find Key name=" << key.c_str() << " value=" << value.v_string.str << std::endl;
                                                strncpy(tmpConfigInfoHead->node_value, value.v_string.str, MAX_NODE_VALUE_LEN - 1);
                                                break;
                                            }
                                        } else if (value.typeId == ALLJOYN_ARRAY && value.Signature().compare("as") == 0) {
                                            std::cout << "Key name=" << key.c_str() << " values: ";
                                            const MsgArg* stringArray;
                                            size_t fieldListNumElements;
                                            status = value.Get("as", &fieldListNumElements, &stringArray);
                                            for (unsigned int i = 0; i < fieldListNumElements; i++) {
                                                char* tempString;
                                                stringArray[i].Get("s", &tempString);
                                                std::cout << tempString << " ";
                                            }
                                            std::cout << std::endl;
                                        }
                                    }
                                } else {
                                    std::cout << "WARNING - Call to GetConfigurations failed: " << QCC_StatusText(status) << std::endl;
                                }
                            }
                        }
                        tmpConfigInfoHead = tmpConfigInfoHead->next;
                    }
                    break;

                case Set:
                    std::cout << "set" << std::endl;
                    for(int i = 0; i < keyNameNumber ; i ++)
                    {
                        if(tmpConfigInfoHead->node_name_convert == NULL)
                        {
                            std::cout << "The keyName you want to set its value is NULL, please recheck" << std::endl;;
                            continue;
                        }
                        else
                        {
                            if (!s_interrupt) {
                                if(strcmp(tmpConfigInfoHead->node_name_convert, "Test") == 0)
                                {
                                    if (!s_interrupt) {
                                        std::cout << "\nGoing to call to UpdateConfigurations: key=Test value=" << tmpConfigInfoHead->node_value << std::endl;
                                        std::cout << "-------------------------------------------------------------------" << std::endl;
                                        services::NewConfigClient::Configurations updateConfigurations;
                                        updateConfigurations.insert(
                                                std::pair<qcc::String, ajn::MsgArg>(tmpConfigInfoHead->node_name_convert, MsgArg("s", tmpConfigInfoHead->node_value)));
                                        if ((status = newConfigClient->UpdateConfigurations(busName.c_str(), NULL, updateConfigurations, objectPath, interfaceName, id)) == ER_OK) {
                                            std::cout << "UpdateConfigurations succeeded" << std::endl;
                                            tmpConfigInfoHead->status = 0;
                                        } else {
                                            std::cout << "WARNING - Call to UpdateConfigurations failed: " << QCC_StatusText(status) << std::endl;
                                            memset(tmpConfigInfoHead->node_value, 0, MAX_NODE_VALUE_LEN);
                                            strcpy(tmpConfigInfoHead->node_value, "setError");
                                            tmpConfigInfoHead->status = 1;
                                        }

                                        //printAllAboutData(aboutProxy);
                                        interruptibleDelay(3);
                                    }
                                }
                                else
                                {

                                    std::cout << "\nGoing to call to UpdateConfigurations: set keyName = " << tmpConfigInfoHead->node_name_convert << ", value = " << tmpConfigInfoHead->node_value << std::endl;
                                    std::cout << "-----------------------------------------------------------------------------------------------" << std::endl;
                                    services::NewConfigClient::Configurations updateConfigurations;
                                    updateConfigurations.insert(
                                            std::pair<qcc::String, ajn::MsgArg>(tmpConfigInfoHead->node_name_convert,
                                                MsgArg("s", tmpConfigInfoHead->node_value)));

                                    if ((status = newConfigClient->UpdateConfigurations(busName.c_str(), "en", updateConfigurations, objectPath, interfaceName, id)) == ER_OK) {
                                        std::cout << "UpdateConfigurations succeeded" << std::endl;
                                        tmpConfigInfoHead->status = 0;
                                    } else {
                                        std::cout << "WARNING - Call to UpdateConfigurations failed: " << QCC_StatusText(status) << std::endl;
                                        memset(tmpConfigInfoHead->node_value, 0, MAX_NODE_VALUE_LEN);
                                        strcpy(tmpConfigInfoHead->node_value, "setError");
                                        tmpConfigInfoHead->status = 1;
                                    }
                                    //printAllAboutData(aboutProxy);
                                }
                            }
                            tmpConfigInfoHead = tmpConfigInfoHead->next;
                            interruptibleDelay(3);
                        }
                    }

                    break;

                case Notify:
                    std::cout << "Notify" << std::endl;
                    break;

                default:
                    std::cout << "Unknown msgType" << std::endl;
                    break;
            }

        } //if (newConfigClient)
    } //if (isConfigInterface)

    status = busAttachment->LeaveSession(id);
    std::cout << "Leaving session id = " << id << " with " << busName.c_str() << " status: " << QCC_StatusText(status) << std::endl;

    if (newConfigClient) {
        delete newConfigClient;
        newConfigClient = NULL;
    }

    s_stopped = true;
    callbackFinish = 1;
    std::cout<<"GaoJing Exit UpdateConfig func"<<std::endl;
}

int InteresteForAllConfigInterfaces(const char **interfaces)
{
    std::cout << "GaoJing Enter interest func" << std::endl;
    QStatus status = ER_OK;

    srpKeyXListener = new SrpKeyXListener();
    srpKeyXListener->setPassCode(INITIAL_PASSCODE);

    status = busAttachment->EnablePeerSecurity("ALLJOYN_SRP_KEYX ALLJOYN_PIN_KEYX ALLJOYN_ECDHE_PSK", srpKeyXListener,
            "/.alljoyn_keystore/central.ks", true);

    if (ER_OK == status) {
        std::cout << "ER_BUS_SECURITY__ENABLED" << std::endl;
    } else {
        std::cout << "ERROR - ER_BUS_SECURITY_NOT_ENABLED " << QCC_StatusText(status) << std::endl;
        return 1;
    }

    aboutListener = new MyAboutListener();
    busAttachment->RegisterAboutListener(*aboutListener);

    status = busAttachment->WhoImplements(interfaces, sizeof(interfaces) / sizeof(interfaces[0]));
    if (ER_OK == status) {
        std::cout << "WhoImplements called." << std::endl;
    } else {
        std::cout << "ERROR - WhoImplements call FAILED with status " << QCC_StatusText(status) << std::endl;
        return 1;
    }

#if 0
    std::cout<<"GaoJing line 691"<<std::endl;

    WaitForSigInt();

    std::cout << "Preparing to exit..." << std::endl;

    if (!s_stopped) {
        std::cout << "Waiting for a few seconds for commands to complete... " << std::endl;

        for (int i = 0; !s_stopped && i < 5; i++) {
#ifdef _WIN32
            Sleep(1000);
#else
            usleep(1000 * 1000);
#endif
        }
    }

    std::cout << "Cleaning up (press Ctrl-C to abort)... " << std::endl;


    busAttachment->CancelWhoImplements(interfaces, sizeof(interfaces) / sizeof(interfaces[0]));
    busAttachment->UnregisterAboutListener(*aboutListener);
    busAttachment->EnablePeerSecurity(NULL, NULL, NULL, true);

    delete srpKeyXListener;
    delete aboutListener;
    srpKeyXListener = 0;
    aboutListener = 0;

    busAttachment->Stop();
    delete busAttachment;
#endif

    std::cout << "GaoJing Exit interest func" << std::endl;

    return 0;
}

/* notification add by gaojing start 2016.06.28 */
int NotificationInit()
{
    std::cout << "Begin Notification Consumer Application." << std::endl;
    QStatus status = ER_OK;
    std::string listOfApps = "";  //ConfigServiceApp
    /* Set a list of app names (separated by ';') you would like to receive notifications from.
     * Empty list means all app names.
     */

    /* Initialize Service object and send it Notification Receiver object
    */
    conService = NotificationService::getInstance();
    Receiver = new NotificationReceiverTestImpl(false);

    /* Set the list of applications this receiver should receive notifications from
    */
    Receiver->setApplications(listOfApps.c_str());

    status = conService->initReceive(busAttachment, Receiver);
    if (status != ER_OK) {
        std::cout << "Could not initialize receiver." << std::endl;
        conService->shutdown();
        conService = NULL;
        delete Receiver;
        Receiver = NULL;
        return 1;
    }

    std::cout << "Waiting for notifications." << std::endl;
    return 0;
}
/* notification add by gaojing end 2016.06.28 */

void StartAlljoynService()
{
    std::cout << "GaoJing Enter StartAlljoynService func" << std::endl;
    CreateBusAttachment();
    const char* interfaces[] = {"org.alljoyn.*.Config"};
    InteresteForAllConfigInterfaces(interfaces);
    NotificationInit();
    std::cout << "GaoJing Exit StartAlljoynService func" << std::endl;
}

void StopAlljoynService()
{
    std::cout << "GaoJing Enter StopAlljoynService func" << std::endl;
    const char* interfaces[] = {"org.alljoyn.*.Config"};
    busAttachment->CancelWhoImplements(interfaces, sizeof(interfaces) / sizeof(interfaces[0]));
    busAttachment->UnregisterAboutListener(*aboutListener);
    busAttachment->EnablePeerSecurity(NULL, NULL, NULL, true);

    delete srpKeyXListener;
    delete aboutListener;
    srpKeyXListener = 0;
    aboutListener = 0;

    busAttachment->Stop();
    delete busAttachment;
    busAttachment = 0;
    std::cout << "GaoJing Exit StopAlljoynService func" << std::endl;
    /* notification add by gaojing start */
    cleanup();
    /* notification add by gaojing end */
}


int configClientMain(char *interfaces, char *matchObjectPath, int msgType, struct ipk_info *configInfoHead, int keyNameNumber)
{
    std::cout << "GaoJing Enter configClientMain func" << std::endl;
    if(busAttachment == 0)
    {
        std::cout << "GaoJing Exit configClientMain func" << std::endl;
        return -1;
    }
    std::cout <<"busAttachment = " << busAttachment << ", interfaces = " << interfaces << ", matchObjectPath = " << matchObjectPath << "" <<std::endl;
    remoteConfigFlag = 1;
    interfaceName = interfaces;
    objectPath = matchObjectPath;
    getMsgType = msgType;
    getKeyNameNumber = keyNameNumber;
    getConfigInfoHead = configInfoHead;

    HandleUpdateConfigMethod(interfaces);

    while(callbackFinish != 1)
    {
        usleep(100*1000);
    }
    s_interrupt = false;
    s_stopped = false;
    callbackFinish = 0;
    std::cout << "GaoJing Exit configClientMain func" << std::endl;
    return 0;

} /* main() */


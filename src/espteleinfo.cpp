#include "espteleinfo.h"

#define CLIENT_ID "EdfTeleinfoKit"
#define INTERVAL 3000 // 3 sec delay between publishing

#define IINST "IINST"
#define PAPP "PAPP"
#define HC "HCHC"
#define HP "HCHP"
#define ADCO "ADCO"
#define OPTARIF "OPTARIF"
#define ISOUSC "ISOUSC"
#define IMAX "IMAX"
#define PTEC "PTEC"

#define NBTRY   5

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

TeleInfo teleinfo(&Serial);

ESPTeleInfo::ESPTeleInfo()
{
    mqtt_user[0] = '\0';
    mqtt_pwd[0] = '\0';
}

void ESPTeleInfo::init()
{
    staticInfoSsent = false;
    previousMillis = millis();
    iinst = 0;
    iinst_old = 254;
    papp = 0;
    papp_old = 1;
    hc = 0;
    hc_old = 1;
    hp = 0;
    hp_old = 1;
    imax = 0;
    imax_old = 1;
    isousc = 0;
    isousc_old = 1;
    adc0[0] = '\0';
    ptec[0] = '\0';
    ptec_old[0] = '_';
    
    Serial.begin(1200, SERIAL_8N1);
    // Init teleinfo
    teleinfo.begin();
}

void ESPTeleInfo::initMqtt(char *server, uint16_t port, char *username, char *password)
{
    strcpy(mqtt_user, username);
    strcpy(mqtt_pwd, password);

    mqttClient.setServer(server, port);
}

bool ESPTeleInfo::connectMqtt(){
    if(mqtt_user[0] == '\0')
    {
        return mqttClient.connect(CLIENT_ID);
    }
    else{
        return mqttClient.connect(CLIENT_ID, mqtt_user, mqtt_pwd);
    }
}

void ESPTeleInfo::loop(void)
{
    teleinfo.process();
    if (teleinfo.available())
    {
        iinst = teleinfo.getLongVal(IINST);
        papp = teleinfo.getLongVal(PAPP);
        hc = teleinfo.getLongVal(HC);
        hp = teleinfo.getLongVal(HP);
        imax = teleinfo.getLongVal(IMAX);
        strncpy(ptec, teleinfo.getStringVal(PTEC), 20);

        if (connectMqtt())
        {
            if (iinst != iinst_old)
            {
                mqttClient.publish("edf/iinst", teleinfo.getStringVal(IINST));
            }
            if (papp != papp_old)
            {
                mqttClient.publish("edf/papp", teleinfo.getStringVal(PAPP));
            }
            if (hc != hc_old && hc != 0)
            {
                mqttClient.publish("edf/hc", teleinfo.getStringVal(HC));
            }
            if (hp != hp_old && hp != 0)
            {
                mqttClient.publish("edf/hp", teleinfo.getStringVal(HP));
            }
            if (imax != imax_old)
            {
                mqttClient.publish("edf/imax", teleinfo.getStringVal(IMAX));
            }
            if (strcmp(ptec, ptec_old) != 0)
            {
                mqttClient.publish("edf/ptec", teleinfo.getStringVal(PTEC));
            }
        }

        iinst_old = iinst;
        papp_old = papp;
        hc_old = hc;
        hp_old = hp;
        imax_old = imax;
        strncpy(ptec_old, ptec, 20);

        if (!staticInfoSsent)
        {
            if (teleinfo.getStringVal(ADCO)[0] != '\n')
            {
                strncpy(adc0, teleinfo.getStringVal(ADCO), 20);
                mqttClient.publish("edf/adc0", teleinfo.getStringVal(ADCO), true);
            }
            if (teleinfo.getStringVal(ISOUSC)[0] != '\n')
            {
                isousc = teleinfo.getLongVal(ISOUSC);
                mqttClient.publish("edf/isousc", teleinfo.getStringVal(ISOUSC), true);
            }

            staticInfoSsent = true;
        }

        teleinfo.resetAvailable();
    }
}

bool ESPTeleInfo::LogStartup()
{
    int8_t nbTry = 0;
    while (nbTry < NBTRY && !connectMqtt())
    {
        delay(250);
        nbTry++;
    }
    if(nbTry < NBTRY){
        mqttClient.publish("edf/log", "Startup");
        return true;
    }
    else{
        return false;
    }
}

// 30 char max !
void ESPTeleInfo::Log(String s)
{
    int8_t nbTry = 0;
    while (nbTry < NBTRY && !connectMqtt())
    {
        delay(250);
        nbTry++;
    }
    if(nbTry < NBTRY){
        s.toCharArray(buffer, 30);
        mqttClient.publish("edf/log", buffer);
    }
}
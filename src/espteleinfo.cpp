#include "espteleinfo.h"

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

#define NBTRY 5

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


    sprintf(CHIP_ID, "%06X", ESP.getChipId());


}

void ESPTeleInfo::initMqtt(char *server, uint16_t port, char *username, char *password, int period_data_power, int period_data_index)
{
    strcpy(mqtt_user, username);
    strcpy(mqtt_pwd, password);

    delay_index = period_data_index;
    delay_power = period_data_power;

    mqttClient.setServer(server, port);
}

bool ESPTeleInfo::connectMqtt()
{
    if (mqtt_user[0] == '\0')
    {
        return mqttClient.connect(CHIP_ID);
    }
    else
    {
        return mqttClient.connect(CHIP_ID, mqtt_user, mqtt_pwd);
    }
}

void ESPTeleInfo::loop(void)
{
    teleinfo.process();
    
    if (teleinfo.available())
    {
        sendPower = sendPowerData();
        sendIndex = sendIndexData();
        
        if(sendPower)
        {
            iinst = teleinfo.getLongVal(IINST);
            papp = teleinfo.getLongVal(PAPP);
        }

        if(sendIndex)
        {
            hc = teleinfo.getLongVal(HC);
            hp = teleinfo.getLongVal(HP);
        }

        imax = teleinfo.getLongVal(IMAX);
        strncpy(ptec, teleinfo.getStringVal(PTEC), 20);

        if (connectMqtt())
        {
            if (iinst != iinst_old && sendPower)
            {
                mqttClient.publish("teleinfokit/iinst", teleinfo.getStringVal(IINST));
            }
            if (papp != papp_old && sendPower)
            {
                mqttClient.publish("teleinfokit/papp", teleinfo.getStringVal(PAPP));
            }
            if (hc != hc_old && hc != 0 && sendIndex)
            {
                mqttClient.publish("teleinfokit/hc", teleinfo.getStringVal(HC));
            }
            if (hp != hp_old && hp != 0 && sendIndex)
            {
                mqttClient.publish("teleinfokit/hp", teleinfo.getStringVal(HP));
            }
            if (imax != imax_old)
            {
                mqttClient.publish("teleinfokit/imax", teleinfo.getStringVal(IMAX));
            }
            if (strcmp(ptec, ptec_old) != 0)
            {
                mqttClient.publish("teleinfokit/ptec", teleinfo.getStringVal(PTEC));
            }
        }

        if(sendPower)
        {
            iinst_old = iinst;
            papp_old = papp;
            ts_power = millis();
        }

        if(sendIndex){
            hc_old = hc;
            hp_old = hp;
            ts_index = millis();
        }

        imax_old = imax;
        strncpy(ptec_old, ptec, 20);

        if (!staticInfoSsent)
        {
            if (teleinfo.getStringVal(ADCO)[0] != '\n')
            {
                strncpy(adc0, teleinfo.getStringVal(ADCO), 20);
                mqttClient.publish("teleinfokit/adc0", teleinfo.getStringVal(ADCO), true);
            }
            if (teleinfo.getStringVal(ISOUSC)[0] != '\n')
            {
                isousc = teleinfo.getLongVal(ISOUSC);
                mqttClient.publish("teleinfokit/isousc", teleinfo.getStringVal(ISOUSC), true);
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
    if (nbTry < NBTRY)
    {
        char str[80];
        mqttClient.publish("teleinfokit/log", "Startup");
        strcpy (str,"Version: ");
        strcat (str, VERSION);
        mqttClient.publish("teleinfokit/log", str);
    #ifdef _HW_VER
        sprintf(str, "HW Version: %d", _HW_VER);
        mqttClient.publish("teleinfokit/log", str);
    #endif
        strcpy (str,"IP: ");
        strcat (str, WiFi.localIP().toString().c_str());
        mqttClient.publish("teleinfokit/log", str);
        strcpy (str,"MAC: ");
        strcat (str, WiFi.macAddress().c_str());
        mqttClient.publish("teleinfokit/log", str);
        return true;
    }
    else
    {
        return false;
    }
}

bool ESPTeleInfo::sendPowerData()
{
    return (delay_power <= 0 ) || (millis() - ts_power > (delay_power));
}

bool ESPTeleInfo::sendIndexData()
{
    return (delay_index <= 0 ) || (millis() - ts_index > (delay_index));
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
    if (nbTry < NBTRY)
    {
        s.toCharArray(buffer, 30);
        mqttClient.publish("teleinfokit/log", buffer);
    }
}

#include "mqtt_handler.h"

WiFiClientSecure espClient;
PubSubClient client(espClient);
String mqtt_base = "SENG3030/Thursday/" + String(deviceId) + "/";
String mqtt_controller_base = "SENG3030/Thursday/" + String(controllerId) + "/" + String(deviceId) + "/";

const char* ca_cert = R"EOF(
-----BEGIN CERTIFICATE-----
MIIDrzCCApegAwIBAgIQCDvgVpBCRrGhdWrJWZHHSjANBgkqhkiG9w0BAQUFADBh
MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3
d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBD
QTAeFw0wNjExMTAwMDAwMDBaFw0zMTExMTAwMDAwMDBaMGExCzAJBgNVBAYTAlVT
MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAXBgNVBAsTEHd3dy5kaWdpY2VydC5j
b20xIDAeBgNVBAMTF0RpZ2lDZXJ0IEdsb2JhbCBSb290IENBMIIBIjANBgkqhkiG
9w0BAQEFAAOCAQ8AMIIBCgKCAQEA4jvhEXLeqKTTo1eqUKKPC3eQyaKl7hLOllsB
CSDMAZOnTjC3U/dDxGkAV53ijSLdhwZAAIEJzs4bg7/fzTtxRuLWZscFs3YnFo97
nh6Vfe63SKMI2tavegw5BmV/Sl0fvBf4q77uKNd0f3p4mVmFaG5cIzJLv07A6Fpt
43C/dxC//AH2hdmoRBBYMql1GNXRor5H4idq9Joz+EkIYIvUX7Q6hL+hqkpMfT7P
T19sdl6gSzeRntwi5m3OFBqOasv+zbMUZBfHWymeMr/y7vrTC0LUq7dBMtoM1O/4
gdW7jVg/tRvoSSiicNoxBN33shbyTApOB6jtSj1etX+jkMOvJwIDAQABo2MwYTAO
BgNVHQ8BAf8EBAMCAYYwDwYDVR0TAQH/BAUwAwEB/zAdBgNVHQ4EFgQUA95QNVbR
TLtm8KPiGxvDl7I90VUwHwYDVR0jBBgwFoAUA95QNVbRTLtm8KPiGxvDl7I90VUw
DQYJKoZIhvcNAQEFBQADggEBAMucN6pIExIK+t1EnE9SsPTfrgT1eXkIoyQY/Esr
hMAtudXH/vTBH1jLuG2cenTnmCmrEbXjcKChzUyImZOMkXDiqw8cvpOp/2PV5Adg
06O/nVsJ8dWO41P0jmP6P6fbtGbfYmbW0W5BjfIttep3Sp+dWOIrWcBAI+0tKIJF
PnlUkiaY4IBIqDfv8NZ5YBberOgOzW6sRBc4L0na4UU+Krk2U886UAb3LujEV0ls
YSEY1QSteDwsOoBrp+uvFRTp2InBuThs4pFsiv9kuXclVzDAGySj4dzp30d8tbQk
CAUw7C29C79Fv1C5qfPrmAESrciIxpg0X40KPMbp1ZWVbd4=
-----END CERTIFICATE-----
)EOF";

//Helper function to split a string on a delimiter
int splitString(String str, char delimiter, String* output, int maxParts) {
    int partIndex = 0;
    int startIndex = 0;
    int delimIndex;
    
    while ((delimIndex = str.indexOf(delimiter, startIndex)) != -1 && partIndex < maxParts - 1) {
        output[partIndex++] = str.substring(startIndex, delimIndex);
        startIndex = delimIndex + 1;
    }
    
    //Add the last part (or the whole string if no delimiter found)
    if (partIndex < maxParts) {
        output[partIndex++] = str.substring(startIndex);
    }
    
    return partIndex;  //Returns the number of parts
}

void setup_wifi() {
    Serial.println("Connecting to WiFi...");
    WiFi.begin(ssid, password);
    
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    
    Serial.println("\nWiFi connected!");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
}

void initMQTT() {
    espClient.setCACert(ca_cert);
    client.setServer(mqtt_broker, mqtt_port);
    client.setBufferSize(16384);
    client.setCallback(mqttCallback);
}

void connectToMQTTBroker() {
    while (!client.connected()) {
        String client_id = "esp32-" + String(WiFi.macAddress());
        Serial.println("Connecting to MQTT...");
        
        if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
            Serial.println("MQTT Connected");

            //Jrice1041/subscribe

            //Base: SENG3030/Thursday/Jrice1041/
            //Post: SENG3030/Thursday/<CTRLID>/Jrice1041/subscribe
            String subTopic = String(mqtt_controller_base) + "subscribe";
            String unsubTopic = String(mqtt_controller_base) + "unsubscribe";

            client.subscribe(subTopic.c_str());
            client.subscribe(unsubTopic.c_str());

            Serial.print("Listening for control on: ");
            Serial.println(subTopic);
            Serial.print("Listening for control on: ");
            Serial.println(unsubTopic);

        } else {
            Serial.print("Failed, rc=");
            Serial.println(client.state());
            delay(5000);
        }
    }
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
    String topicStr(topic);

    bool isMicBytes = topicStr.endsWith("/mic/bytes");

    String message;
    if (!isMicBytes) {
        message.reserve(length);
        for (unsigned int i = 0; i < length; i++) {
            message += (char)payload[i];
        }
    }

    Serial.print("Message on topic: ");
    Serial.println(topicStr);

    // Split topic on '/' to get an array of topic parts
    String topicParts[10];
    int numParts = splitString(topicStr, '/', topicParts, 10);


    //Can't be less than 4 parts
    if(numParts < 4){
        Serial.println("ERR:mqttCallback: Bad topic received:");
        Serial.println(topicStr);
        return;
    }

    //Part[0]: junk
    //Part[1]: junk
    //Part[2]: CTRL: controller ID      ||  device ID
    //Part[3]: CTRL: device ID          ||  battery, or subtopic
    //Part[4]: CTRL: Command            ||  value

    //IF MESSAGE IS FROM CONTROL DEVICE
    if(topicParts[2] == String(controllerId)){
        String command = topicParts[4];
        String targetDevice = topicParts[3];
        String targetTopic = message;

        //Controller good, command bad.
        if(command != "subscribe" && command != "unsubscribe"){
            Serial.println("ERR:mqttCallback: Received message from controller. Command is unrecognized. Full topic string:");
            Serial.println(topicStr);
            return;
        }

        if(targetTopic == ""){
            Serial.println("ERR:mqttCallback: Received message from controller. Target device is empty.");
            return;
        }

        //Subscribe || unsubscribe to given device ID
        if (command == "subscribe") {
            Serial.print("Control from ");
            Serial.print(controllerId);
            Serial.print("Subscribe to: ");
            Serial.println(targetTopic);

            if (client.subscribe(targetTopic.c_str())) {
                Serial.println("Subscribe success");
            } else {
                Serial.println("Subscribe failed");
            }
        } 
        else {
            Serial.print("Control from ");
            Serial.print(controllerId);
            Serial.print("Unsubscribe from: ");
            Serial.println(targetTopic);

            if (client.unsubscribe(targetTopic.c_str())) {
                Serial.println("Unsubscribe success");
            } else {
                Serial.println("Unsubscribe failed");
            }
        }
        return;
    }


    //Make beautiful music
    if (isMicBytes) {
        Serial.println("Playing mic audio");
        play_recording((char*)payload, length);
        return;
    }

    //Pass in DeviceID/subtopic name
    String subtopicName = topicParts[2] + "/" + topicParts[3];

    //Pass in nested subtopic name (if exists.)
    if(!topicParts[4].isEmpty()){
        subtopicName = subtopicName + "/" + topicParts[4];
    }

    //Display to GUI
    handleIncomingMqttMessage(subtopicName, message);
}





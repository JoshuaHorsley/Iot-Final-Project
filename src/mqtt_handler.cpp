#include "mqtt_handler.h"

WiFiClientSecure espClient;
PubSubClient client(espClient);
String mqtt_base = "SENG3030/Thursday/" + String(deviceId) + "/";

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

void setup_wifi() {
    Serial.println("Connecting to WiFi...");
    M5.Display.println("Connecting WiFi...");
    WiFi.begin(ssid, password);
    
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
        M5.Display.print(".");
    }
    
    Serial.println("\nWiFi connected!");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
    M5.Display.println("\nWiFi Connected!");
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
        M5.Display.println("Connecting MQTT...");
        
        if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
            Serial.println("MQTT Connected!");
            M5.Display.println("MQTT Connected!");
            client.publish((mqtt_base + "status").c_str(), "device_online");

            String subTopic = "+/" + String(deviceId) + "/subscribe";
            String unsubTopic = "+/" + String(deviceId) + "/unsubscribe";

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

    String message = "";
    message.reserve(length);
    for (unsigned int i = 0; i < length; i++) {
        message += (char)payload[i];
    }

    Serial.print("Message on topic: ");
    Serial.println(topicStr);
    Serial.print("Payload: ");
    Serial.println(message);

    int firstSlash = topicStr.indexOf('/');
    if (firstSlash < 0) {
        handleIncomingMqttMessage(topicStr, message);
        return;
    }

    int secondSlash = topicStr.indexOf('/', firstSlash + 1);
    if (secondSlash < 0) {
        handleIncomingMqttMessage(topicStr, message);
        return;
    }

    String controllerUserId = topicStr.substring(0, firstSlash);
    String targetDeviceId = topicStr.substring(firstSlash + 1, secondSlash);
    String command = topicStr.substring(secondSlash + 1);

    if (targetDeviceId == String(deviceId) && (command == "subscribe" || command == "unsubscribe")) {
        String targetTopic = message;
        targetTopic.trim();

        if (command == "subscribe") {
            Serial.print("Control from ");
            Serial.print(controllerUserId);
            Serial.print(" -> SUBSCRIBE to: ");
            Serial.println(targetTopic);

            if (client.subscribe(targetTopic.c_str())) {
                Serial.println("Subscribe success");
            } else {
                Serial.println("Subscribe FAILED");
            }
        } 
        else if (command == "unsubscribe") {
            Serial.print("Control from ");
            Serial.print(controllerUserId);
            Serial.print(" -> UNSUBSCRIBE from: ");
            Serial.println(targetTopic);

            if (client.unsubscribe(targetTopic.c_str())) {
                Serial.println("Unsubscribe success");
            } else {
                Serial.println("Unsubscribe FAILED");
            }
        }
        return;
    }

    handleIncomingMqttMessage(topicStr, message);

    // still need to send microphone data to speaker
}
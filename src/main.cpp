#include <M5Unified.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <M5UnitENV.h>
#include "config.h"

// ============== I2C PINS ==============
#define EXT_SDA 32
#define EXT_SCL 33



// ============== MQTT CONFIG ==============

String mqtt_base = "SENG3030/Thursday/" + String(deviceId) + "/";

// ============== TLS CERTIFICATE ==============
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

// ============== GLOBAL OBJECTS ==============
WiFiClientSecure espClient;
PubSubClient client(espClient);

SHT4X sht4;
BMP280 bmp;
bool env_ok = false;

// ============== STATE VARIABLES ==============
bool isSending = false;                      
unsigned long lastPublish = 0;
const unsigned long publishInterval = 1000; 

char mqtt_buffer[40];

// ============== FUNCTION DECLARATIONS ==============
void setup_wifi(void);
void setup_unit_env(void);
void connectToMQTTBroker(void);
void mqttCallback(char* topic, byte* payload, unsigned int length);
void publishSensorData(void);
void updateBackground(void);
void checkPowerButton(void);

// ============== DISPLAY FUNCTIONS ==============
void updateBackground() {
    M5.Display.fillScreen(isSending ? TFT_GREEN : TFT_BLUE);
    M5.Display.setTextColor(isSending ? TFT_BLACK : TFT_WHITE);
    M5.Display.setTextSize(2);
    
    // Line 1
    M5.Display.setCursor(10, 20);
    M5.Display.println("SENDING:");
    
    // Line 2
    M5.Display.setCursor(10, 45);
    M5.Display.println(isSending ? "ON" : "OFF");
}

// ============== INPUT FUNCTIONS ==============
void checkPowerButton() {
    // Power button single click toggles data sending
    if (M5.BtnB.wasClicked()) {
        isSending = !isSending;
        updateBackground();
        
        Serial.print("Data sending:\n");
        Serial.println(isSending ? "ON" : "OFF");
        
        // Publish status change
        client.publish((mqtt_base + "status").c_str(), 
                       isSending ? "sending_on" : "sending_off");
    }
}

// ============== WIFI FUNCTIONS ==============
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

// ============== MQTT FUNCTIONS ==============
void mqttCallback(char* topic, byte* payload, unsigned int length) {
    Serial.print("Message on: ");
    Serial.println(topic);
    
    String message = "";
    for (unsigned int i = 0; i < length; i++) {
        message += (char)payload[i];
    }
    Serial.print("Message: ");
    Serial.println(message);
    
    // TODO: expand this for subscribe/unsubscribe handling
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
        } else {
            Serial.print("Failed, rc=");
            Serial.println(client.state());
            delay(5000);
        }
    }
}

// ============== SENSOR FUNCTIONS ==============
void setup_unit_env() {
    Serial.println("Setting up ENV sensors...");
    
    if (!sht4.begin(&Wire, SHT40_I2C_ADDR_44, EXT_SDA, EXT_SCL, 400000U)) {
        Serial.println("SHT4x not found");
        return;
    }
    sht4.setPrecision(SHT4X_HIGH_PRECISION);
    sht4.setHeater(SHT4X_NO_HEATER);
    
    if (!bmp.begin(&Wire, BMP280_I2C_ADDR, EXT_SDA, EXT_SCL, 400000U)) {
        Serial.println("BMP280 not found");
        return;
    }
    bmp.setSampling(BMP280::MODE_NORMAL, BMP280::SAMPLING_X2,
                    BMP280::SAMPLING_X16, BMP280::FILTER_X16,
                    BMP280::STANDBY_MS_500);
    
    env_ok = true;
    Serial.println("ENV sensors ready!");
}

void publishSensorData() {
    Serial.println("=== Publishing Sensor Data ===");
    
    // ENV Sensors (SHT40 + BMP280)
    if (env_ok) {
        if (sht4.update()) {
            sprintf(mqtt_buffer, "%.2f", sht4.cTemp);
            client.publish((mqtt_base + "sht40/temperature").c_str(), mqtt_buffer);
            
            sprintf(mqtt_buffer, "%.2f", sht4.humidity);
            client.publish((mqtt_base + "sht40/humidity").c_str(), mqtt_buffer);
        }
        
        if (bmp.update()) {
            sprintf(mqtt_buffer, "%.2f", bmp.cTemp);
            client.publish((mqtt_base + "bmp280/temperature").c_str(), mqtt_buffer);
            
            sprintf(mqtt_buffer, "%.2f", bmp.pressure);
            client.publish((mqtt_base + "bmp280/pressure").c_str(), mqtt_buffer);
        }
    }
    
    // Battery
    int voltage = M5.Power.getBatteryVoltage();
    sprintf(mqtt_buffer, "%d", voltage);
    client.publish((mqtt_base + "battery").c_str(), mqtt_buffer);
    
    // IMU
    if (M5.Imu.update()) {
        auto data = M5.Imu.getImuData();
        
        sprintf(mqtt_buffer, "%.2f,%.2f,%.2f", data.accel.x, data.accel.y, data.accel.z);
        client.publish((mqtt_base + "imu/accel").c_str(), mqtt_buffer);
        
        sprintf(mqtt_buffer, "%.2f,%.2f,%.2f", data.gyro.x, data.gyro.y, data.gyro.z);
        client.publish((mqtt_base + "imu/gyro").c_str(), mqtt_buffer);
    }
    
    Serial.println("=== Done ===\n");
}

// ============== SETUP ==============
void setup() {
    Serial.begin(115200);
    Serial.println("** Starting Setup **");
    
    auto cfg = M5.config();
    M5.begin(cfg);
    M5.Display.setTextSize(2);
    
    // Setup sensors
    setup_unit_env();
    
    // Setup WiFi
    setup_wifi();
    
    // Setup MQTT
    espClient.setCACert(ca_cert);
    client.setServer(mqtt_broker, mqtt_port);
    client.setBufferSize(16384);  // For audio 
    client.setCallback(mqttCallback);
    connectToMQTTBroker();
    
    // Initial display (blue = not sending)
    updateBackground();
    
    Serial.println("** Ready! **");
}

// ============== LOOP ==============
void loop() {
    M5.update();
    
    // Maintain MQTT connection
    if (!client.connected()) {
        connectToMQTTBroker();
    }
    client.loop();
    
    // Check power button toggle
    checkPowerButton();
    
    // Publish every 1 second IF sending is enabled
    if (isSending && (millis() - lastPublish >= publishInterval)) {
        lastPublish = millis();
        publishSensorData();
    }
}
#include "sensors.h"
#include "mqtt_handler.h"
#include <M5Unified.h>
#include <M5UnitENV.h>

#define EXT_SDA 32
#define EXT_SCL 33

static SHT4X sht4;
static BMP280 bmp;
static bool env_ok = false;
static char mqtt_buffer[40];

void setup_sensors() {
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
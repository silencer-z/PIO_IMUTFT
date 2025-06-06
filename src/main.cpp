#include <TFT_eSPI.h> // Graphics and font library for ST7735 driver chip
#include <SPI.h>
#include <Arduino.h>
#include <SensorQMI8658.hpp>
#include "MLX90640.h"


static float mlx90640To[768];
MLX90640 mlx90640(0x33);


TFT_eSPI tft = TFT_eSPI();  // Invoke library, pins defined in User_Setup.h
#define USE_WIRE

#ifndef SENSOR_SDA
#define SENSOR_SDA  18
#endif

#ifndef SENSOR_SCL
#define SENSOR_SCL  17
#endif

#ifndef SENSOR_IRQ
#define SENSOR_IRQ  -1
#endif


#define IMU_CS      5

SensorQMI8658 qmi;
uint32_t timestamp;
IMUdata Acc_data;
IMUdata Gyro_data;

unsigned long microsPerReading, microsPrevious;

/**
 * @brief 将一帧温度数据以 CSV 格式通过串口发送
 *
 * @param data 指向温度数据数组的指针，数组长度为 len
 * @param len  数组长度（例如 768）
 */
void sendFrameCSV(const float* data, size_t len) {
    for (size_t i = 0; i < len; i++) {
      Serial.print(data[i], 2);  // 保留两位小数
      if (i < len - 1) {
        Serial.print(",");       // 数据之间用逗号分隔
      }
    }
    Serial.println();            // 换行符表示该帧结束
  }


void setup()
{
    Wire.begin();
    Wire.setClock(400000); 
    Serial.begin(115200);
    // analogReadResolution(12);
    tft.init();
    // 设置屏幕旋转方向
    tft.setRotation(1);
    // 填充屏幕为黑色
    tft.fillScreen(TFT_BLACK);
    // 设置字体颜色
    tft.setTextColor(TFT_WHITE,TFT_BLACK);
    // 设置文本大小为2倍
    tft.setTextSize(2);
    // 在屏幕上显示文本
    tft.setCursor(0, 0);


    Serial.print("Start MLX check");
    mlx90640.check();
    mlx90640.begin();
    Serial.print("MLX is ready");


    if (!qmi.begin(Wire, QMI8658_L_SLAVE_ADDRESS, SENSOR_SDA, SENSOR_SCL)) {
        Serial.println("Failed to find QMI8658!");
        while (1) {
            delay(1000);
        }
    }
    /* Get chip id*/
    Serial.print("Device ID:");
    Serial.println(qmi.getChipID(), HEX);

    qmi.configAccelerometer(
        /*
         * ACC_RANGE_2G
         * ACC_RANGE_4G
         * ACC_RANGE_8G
         * ACC_RANGE_16G
         * */
        SensorQMI8658::ACC_RANGE_2G,
        /*
         * ACC_ODR_1000Hz
         * ACC_ODR_500Hz
         * ACC_ODR_250Hz
         * ACC_ODR_125Hz
         * ACC_ODR_62_5Hz
         * ACC_ODR_31_25Hz
         * ACC_ODR_LOWPOWER_128Hz
         * ACC_ODR_LOWPOWER_21Hz
         * ACC_ODR_LOWPOWER_11Hz
         * ACC_ODR_LOWPOWER_3H
        * */
        SensorQMI8658::ACC_ODR_1000Hz,
        /*
        *  LPF_MODE_0     //2.66% of ODR
        *  LPF_MODE_1     //3.63% of ODR
        *  LPF_MODE_2     //5.39% of ODR
        *  LPF_MODE_3     //13.37% of ODR
        * */
        SensorQMI8658::LPF_MODE_0);


    qmi.configGyroscope(
        /*
        * GYR_RANGE_16DPS
        * GYR_RANGE_32DPS
        * GYR_RANGE_64DPS
        * GYR_RANGE_128DPS
        * GYR_RANGE_256DPS
        * GYR_RANGE_512DPS
        * GYR_RANGE_1024DPS
        * */
        SensorQMI8658::GYR_RANGE_256DPS,
        /*
         * GYR_ODR_7174_4Hz
         * GYR_ODR_3587_2Hz
         * GYR_ODR_1793_6Hz
         * GYR_ODR_896_8Hz
         * GYR_ODR_448_4Hz
         * GYR_ODR_224_2Hz
         * GYR_ODR_112_1Hz
         * GYR_ODR_56_05Hz
         * GYR_ODR_28_025H
         * */
        SensorQMI8658::GYR_ODR_896_8Hz,
        /*
        *  LPF_MODE_0     //2.66% of ODR
        *  LPF_MODE_1     //3.63% of ODR
        *  LPF_MODE_2     //5.39% of ODR
        *  LPF_MODE_3     //13.37% of ODR
        * */
        SensorQMI8658::LPF_MODE_3);

    qmi.enableGyroscope();
    qmi.enableAccelerometer();

    /* 控制采样率 */
    microsPerReading = 1000000 / 25;
    microsPrevious = micros();

    pinMode(LCD_POWER_ON, OUTPUT);
    pinMode(PIN_11,OUTPUT);
    digitalWrite(LCD_POWER_ON, HIGH);
    digitalWrite(PIN_11, LOW);

    // Print register configuration information
    // qmi.dumpCtrlRegister();
    Serial.println("Read data now...");
}

void loop()
{
    mlx90640.getData(mlx90640To);
    sendFrameCSV(mlx90640To, 768);
    if (micros() - microsPrevious >= microsPerReading) {
        if (qmi.getDataReady()) {
            tft.setCursor(0, 0);
            tft.fillScreen(TFT_BLACK);
            /* 获取6轴原始数据 */
            /* 加速度单位：g；陀螺仪单位：°/s */
            qmi.getAccelerometer(Acc_data.x,Acc_data.y,Acc_data.z);
            qmi.getGyroscope(Gyro_data.x,Gyro_data.y,Gyro_data.z);
            timestamp = millis();

            tft.printf("ACC:\t %.2f\t %.2f\t %.2f\n",Acc_data.x,Acc_data.y,Acc_data.z);
            tft.printf("GYR:\t %.2f\t %.2f\t %.2f\n",Gyro_data.x,Gyro_data.y,Gyro_data.z);
            tft.printf(" > %lu  %.2f *C\n", timestamp, qmi.getTemperature_C());

            Serial.print("$IMU_Data:");Serial.print(timestamp);Serial.print(",");
            Serial.printf("%f,%f,%f,%f,%f,%f\r\n", Acc_data.x,Acc_data.y,Acc_data.z,Gyro_data.x,Gyro_data.y,Gyro_data.z);
        }
        // // 读取 ADC 原始值
        // int raw = analogRead(A0);
        // // 将原始值转换为电压 (0 - VCC)
        // float vOut = raw / 4095.0 * 3.3;

        // // 串口打印
        // Serial.printf("Raw: %4d  Voltage: %.2f V\n", raw, vOut);

        // // 阈值提示
        // if (vOut > 1) {
        //     Serial.println("*** Threshold Exceeded! ***");
        //     digitalWrite(PIN_11,HIGH);
        // }
        // else{
        //     digitalWrite(PIN_11,LOW);
        // }
        microsPrevious = microsPrevious + microsPerReading;
    }
}





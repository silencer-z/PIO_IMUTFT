#ifndef _MLX90640_
#define _MLX90640_


#include <Arduino.h>
#include <Wire.h>
#include "MLX90640_API.h"

class MLX90640 {
public:
    explicit MLX90640(uint8_t i2c_address = 0x33,uint8_t temp_shift=8);

    int begin();
    int check();
    int getData(float* data);
    int getFrameData(uint16_t* frameData);
    float getVdd(uint16_t* frameData);
    float getTa(uint16_t* frameData);
    void getImage(uint16_t* frameData, float* result);
    void calculateTo(uint16_t* frameData, float emissivity, float tr, float* result);

    int setResolution(uint8_t resolution);
    int getCurResolution();
    int setRefreshRate(uint8_t refreshRate);
    int getRefreshRate();

    int getSubPageNumber(uint16_t* frameData);
    int getCurMode();
    int setInterleavedMode();
    int setChessMode();


    /**
     * @brief 将温度值转换为 16 位颜色值
     * 
     * @param temp     温度值
     * @param tempMin  温度映射最小值
     * @param tempMax  温度映射最大值
     * @return uint16_t 16位颜色（TFT_eSPI 库采用的格式）
     */
    static uint16_t getPseudoColor(float temp, float tempMin, float tempMax);

private:
    uint8_t temperatrue;
    uint8_t slaveAddr;
    uint16_t eeData[832];
    paramsMLX90640 params;
};


#endif

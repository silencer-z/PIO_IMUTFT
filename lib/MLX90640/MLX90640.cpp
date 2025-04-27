#include "MLX90640.h"

MLX90640::MLX90640(uint8_t i2c_address,uint8_t temp_shift) {
    slaveAddr = i2c_address;
    temperatrue = temp_shift;
}

uint16_t MLX90640::getPseudoColor(float temp, float tempMin, float tempMax)
{
    // 根据温度线性映射到 0~255 值
    float ratio = (temp - tempMin) / (tempMax - tempMin);
    ratio = constrain(ratio, 0.0, 1.0);
    uint8_t val = (uint8_t)(ratio * 255.0);
    // 构造伪彩色：此处低温用蓝色，高温用红色
    // 可以根据需求修改 r, g, b 的生成方式
    uint8_t r = val;             // 红色随温度升高增强
    uint8_t g = 0;               // 固定为0，也可按需要变化
    uint8_t b = 255 - val;       // 蓝色随温度降低增强

    // 16位颜色转换：
    // 红色5位：r >> 3, 绿色6位：g >> 2, 蓝色5位：b >> 3
    uint16_t color = ((r & 0xF8) << 8)   // 红色部分
                   | ((g & 0xFC) << 3)   // 绿色部分
                   | (b >> 3);           // 蓝色部分
    return color;
}


int MLX90640::begin() {
    int status = MLX90640_DumpEE(slaveAddr, eeData);
    if (status < 0) return status;

    status = MLX90640_ExtractParameters(eeData, &params);
    return status;
}


int MLX90640::check(){
    Wire.beginTransmission(slaveAddr);
    if (Wire.endTransmission() != 0) {
        while (1);
    }
    return 0;
}

int MLX90640::getData(float *data)
{
    for (byte x = 0 ; x < 2 ; x++) { //Read both subpages
        uint16_t mlx90640Frame[834];
        int status = this->getFrameData(mlx90640Frame);
        if (status < 0) {
            return status;
        }

        float vdd = this->getVdd(mlx90640Frame);
        float Ta = this->getTa(mlx90640Frame);

        float tr = Ta - temperatrue; //Reflected temperature based on the sensor ambient temperature
        float emissivity = 0.95;

        this->calculateTo(mlx90640Frame, emissivity, tr, data);
    }
    return 0;
}

int MLX90640::getFrameData(uint16_t* frameData) {
    return MLX90640_GetFrameData(slaveAddr, frameData);
}

float MLX90640::getVdd(uint16_t* frameData) {
    return MLX90640_GetVdd(frameData, &params);
}

float MLX90640::getTa(uint16_t* frameData) {
    return MLX90640_GetTa(frameData, &params);
}

void MLX90640::getImage(uint16_t* frameData, float* result) {
    MLX90640_GetImage(frameData, &params, result);
}

void MLX90640::calculateTo(uint16_t* frameData, float emissivity, float tr, float* result) {
    MLX90640_CalculateTo(frameData, &params, emissivity, tr, result);
}

int MLX90640::setResolution(uint8_t resolution) {
    return MLX90640_SetResolution(slaveAddr, resolution);
}

int MLX90640::getCurResolution() {
    return MLX90640_GetCurResolution(slaveAddr);
}

int MLX90640::setRefreshRate(uint8_t refreshRate) {
    return MLX90640_SetRefreshRate(slaveAddr, refreshRate);
}

int MLX90640::getRefreshRate() {
    return MLX90640_GetRefreshRate(slaveAddr);
}

int MLX90640::getSubPageNumber(uint16_t* frameData) {
    return MLX90640_GetSubPageNumber(frameData);
}

int MLX90640::getCurMode() {
    return MLX90640_GetCurMode(slaveAddr);
}

int MLX90640::setInterleavedMode() {
    return MLX90640_SetInterleavedMode(slaveAddr);
}

int MLX90640::setChessMode() {
    return MLX90640_SetChessMode(slaveAddr);
}



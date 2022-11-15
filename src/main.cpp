#include <Arduino.h>
#include <Wire.h>
#include <iostream>
#include <vector>

#define LOWER_RANGE_LIMIT 0
#define UPPER_RANGE_LIMIT 1519.88

#define STATUS_REG_ADDRESS 0X02

double read_pressure();
void write_reg(byte reg_address, byte value);
byte read_reg(byte reg_address);
std::vector<byte> read_n_reg(byte reg_address, int n_bytes);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
//   Wire.setClock(400000);
  Wire.begin();

}

void loop() {
    // put your main code here, to run repeatedly:
    static int samples = 0;
    static unsigned long t = millis();

    byte status_reg = read_reg(STATUS_REG_ADDRESS);
    bool reading_ready = status_reg & 0x01;
    if (reading_ready){
        read_pressure();
        samples += 1;
    }

    if (millis() - t > 1000){
        Serial.print("samples per second: ");
        Serial.println(samples);
        t = millis();
        samples = 0;
    }

}

byte read_reg(byte reg_address){
    std::vector<byte> data;
    data = read_n_reg(reg_address, 1);

    return data[0];
}

std::vector<byte> read_n_reg(byte reg_address, int n_bytes){
    std::vector<byte> data;
    Wire.beginTransmission(0x6D);
    Wire.write(reg_address);
    Wire.endTransmission();

    Wire.requestFrom(0x6D, n_bytes);
    for (int i = 0; i<n_bytes; i++){
        data.push_back(Wire.read());
    }
    Wire.endTransmission();

    return data;
}

void write_reg(byte reg_address, byte value){
    Wire.beginTransmission(0x6D);
    Wire.write(reg_address);
    Wire.write(value);
    Wire.endTransmission();
}

double read_pressure()
{
    float fadc;
    float uADC;
    float Range;
    double currentPressure;
    unsigned char buf[6];
    unsigned long dat;
    char error;

    // Wire.beginTransmission(0x6D);
    // error = Wire.endTransmission();

    // if (error != 0){
    //     currentPressure = -1;
    // }
    // else{
    Wire.beginTransmission(0x6D);
    Wire.write(0x06);
    Wire.endTransmission();

    Wire.requestFrom(0x6D, 6);
    buf[0] = Wire.read();
    buf[1] = Wire.read();
    buf[2] = Wire.read();
    buf[3] = Wire.read();
    buf[4] = Wire.read();
    buf[5] = Wire.read();
    Wire.endTransmission();

    dat = buf[0];
    dat <<= 8;
    dat += buf[1];
    dat <<= 8;
    dat += buf[2];
    if (dat & 0x800000)
    {
        fadc = dat - 16777216.0;
    }
    else
    {
        fadc = dat;
    }
    uADC = 3.3 * fadc / 8388608.0;
    Range = UPPER_RANGE_LIMIT - LOWER_RANGE_LIMIT;
    currentPressure = (Range * (uADC - 0.5) / 2.0) + LOWER_RANGE_LIMIT;
    currentPressure = currentPressure / 101.325 - 0.1;
    // }
    return currentPressure;
}

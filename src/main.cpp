#include <AsyncUDP.h>
#include <BLEBeacon.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <M5StickCPlus.h>
#include <WiFi.h>

#define BEACON_UUID "65432461-1EFE-4ADB-BC7E-9F7F8E27FDC1"
#define TX_POWER   -65
int MAJOR = 1024;
int MINOR;
int colors[] = {
    56,
    251,
    504,
    480,
    448,
    455,
    7,
    127,
    2559
};

BLEAdvertising *pAdvertising;

const char *ssid = "M5StickC-Plus-Controller";
const char *password = "controller";

int port = 8888;
AsyncUDP udp;

float acc[3];           

float roll  = 0.0F;         
float pitch = 0.0F;             

const float pi = 3.14;


void drawDisplay(String message) {
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setCursor(15, 50);
    M5.Lcd.print(message);
}


std::string setUUID() {
    std::string org = BEACON_UUID;
    std::string dst = "";
    if (org.length() != 36) {
        return "error";
    }
    dst  = org[34]; dst += org[35];
    dst += org[32]; dst += org[33];
    dst += org[30]; dst += org[31];
    dst += org[28]; dst += org[29];
    dst += org[8];
    dst += org[26]; dst += org[27];
    dst += org[24]; dst += org[25];
    dst += org[23];
    dst += org[21]; dst += org[22];
    dst += org[19]; dst += org[20];
    dst += org[18];
    dst += org[16]; dst += org[17];
    dst += org[14]; dst += org[15];
    dst += org[13];
    dst += org[11]; dst += org[12];
    dst += org[9];  dst += org[10];
    dst += org[6];  dst += org[7];
    dst += org[4];  dst += org[5];
    dst += org[2];  dst += org[3];
    dst += org[0];  dst += org[1];
    return dst;
}


void setBeacon() {
    BLEBeacon oBeacon = BLEBeacon();
    oBeacon.setManufacturerId(0x4C00);
    oBeacon.setProximityUUID(BLEUUID(setUUID()));
    oBeacon.setMajor(MAJOR);
    oBeacon.setMinor(MINOR);
    oBeacon.setSignalPower(TX_POWER);
    BLEAdvertisementData oAdvertisementData = BLEAdvertisementData();
    BLEAdvertisementData oScanResponseData = BLEAdvertisementData();

    oAdvertisementData.setFlags(0x04);

    std::string strServiceData = "";

    strServiceData += (char)26;     // Len
    strServiceData += (char)0xFF;   // Type
    strServiceData += oBeacon.getData(); 
    oAdvertisementData.addData(strServiceData);

    pAdvertising->setAdvertisementData(oAdvertisementData);
    pAdvertising->setScanResponseData(oScanResponseData);
}


void setColor(int color) {
    MINOR = color;
    setBeacon();
    pAdvertising->start();
    //delay(100);
}


void readGyro() {
    M5.IMU.getAccelData(&acc[0], &acc[1], &acc[2]);
    roll  =  atan(acc[0] / sqrt((acc[1] *acc[1]) + (acc[2] * acc[2]))) * 180 / pi; 
    pitch =  atan(acc[1] / sqrt((acc[0] *acc[0]) + (acc[2] * acc[2]))) * 180 / pi;
    if (-20 <= roll && roll <= 20 && -20 <= pitch && pitch <= 20) {
        drawDisplay("Stop");
        udp.broadcastTo("K", port);
        setColor(0);
    } else if (roll < -20 && -20 <= pitch && pitch <= 20) {
        drawDisplay("Forward");
        if (roll < -50) {
            udp.broadcastTo("A", port);
        } else {
            udp.broadcastTo("a", port);
        }
        setColor(colors[0]);
    } else if (20 < roll && -20 <= pitch && pitch <= 20) {
        drawDisplay("Back");
        if (50 < roll) {
            udp.broadcastTo("B", port);
        } else {
            udp.broadcastTo("b", port);
        }
        setColor(colors[4]);
    } else if (-20 <= roll && roll <= 20 && pitch < -20) {
        drawDisplay("Left");
        if (pitch < -50) {
            udp.broadcastTo("G", port);
        } else {
            udp.broadcastTo("g", port);
        }
        setColor(colors[6]);
    } else if (-20 <= roll && roll <= 20 && 20 < pitch) {
        drawDisplay("Right");
        if (50 < pitch) {
            udp.broadcastTo("H", port);
        } else {
            udp.broadcastTo("h", port);
        }
        setColor(colors[2]);
    } else if (roll < -20 && pitch < -20) {
        drawDisplay("Left-Forward");
        if (roll < -50 || pitch < -50) {
            udp.broadcastTo("C", port);
        } else {
            udp.broadcastTo("c", port);
        }
        setColor(colors[7]);
    } else if (roll < -20 && 20 < pitch) {
        drawDisplay("Right-Forward");
        if (roll < -50 || 50 < pitch) {
            udp.broadcastTo("D", port);
        } else {
            udp.broadcastTo("d", port);
        }
        setColor(colors[1]);
    } else if (20 < roll && pitch < -20) {
        drawDisplay("Left-Back");
        if (50 < roll || pitch < -50) {
            udp.broadcastTo("E", port);
        } else {
            udp.broadcastTo("e", port);
        }
        setColor(colors[5]);
    } else if (20 < roll && 20 < pitch) {
        drawDisplay("Right-Back");
        if (50 < roll || 50 < pitch) {
            udp.broadcastTo("F", port);
        } else {
            udp.broadcastTo("f", port);
        }
        setColor(colors[3]);
    }
}


void setup() {
    M5.begin();
    M5.Imu.Init();

    M5.Lcd.setRotation(1);
    M5.Lcd.setTextFont(4);
    M5.Lcd.setTextSize(1);

    drawDisplay("Stop");

    WiFi.mode(WIFI_AP);
    WiFi.softAP(ssid, password);

    BLEDevice::init("");
    pAdvertising = BLEDevice::getAdvertising();

    setBeacon();
}


void loop() {
    M5.update();
    if (M5.BtnA.isReleased() && M5.BtnB.isReleased()) {
        readGyro();
    } else if (M5.BtnA.isPressed() && M5.BtnB.isReleased()) {
        drawDisplay("Button A");
        udp.broadcastTo("I", port);
        setColor(colors[8]);
    } else if (M5.BtnA.isReleased() && M5.BtnB.isPressed()) {
        drawDisplay("Button B");
        udp.broadcastTo("J", port);
        setColor(colors[8]);
    }
    delay(100);
}
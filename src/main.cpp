#include <AsyncUDP.h>
#include <M5StickCPlus.h>
#include <WiFi.h>


const char *ssid = "M5StickC-Controller";
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


void readGyro() {
    M5.IMU.getAccelData(&acc[0], &acc[1], &acc[2]);
    roll  =  atan(acc[0] / sqrt((acc[1] *acc[1]) + (acc[2] * acc[2]))) * 180 / pi; 
    pitch =  atan(acc[1] / sqrt((acc[0] *acc[0]) + (acc[2] * acc[2]))) * 180 / pi;
    if (-20 <= roll && roll <= 20 && -20 <= pitch && pitch <= 20) {
        drawDisplay("Stop");
        udp.broadcastTo("K", port);
    } else if (roll < -20 && -20 <= pitch && pitch <= 20) {
        drawDisplay("Forward");
        if (roll < -50) {
            udp.broadcastTo("A", port);
        } else {
            udp.broadcastTo("a", port);
        }
    } else if (20 < roll && -20 <= pitch && pitch <= 20) {
        drawDisplay("Back");
        if (50 < roll) {
            udp.broadcastTo("B", port);
        } else {
            udp.broadcastTo("b", port);
        }
    } else if (-20 <= roll && roll <= 20 && pitch < -20) {
        drawDisplay("Left");
        if (pitch < -50) {
            udp.broadcastTo("G", port);
        } else {
            udp.broadcastTo("g", port);
        }
    } else if (-20 <= roll && roll <= 20 && 20 < pitch) {
        drawDisplay("Right");
        if (50 < pitch) {
            udp.broadcastTo("H", port);
        } else {
            udp.broadcastTo("h", port);
        }
    } else if (roll < -20 && pitch < -20) {
        drawDisplay("Left-Forward");
        if (roll < -40 || pitch < -40) {
            udp.broadcastTo("C", port);
        } else {
            udp.broadcastTo("c", port);
        }
    } else if (roll < -20 && 20 < pitch) {
        drawDisplay("Right-Forward");
        if (roll < -40 || 40 < pitch) {
            udp.broadcastTo("D", port);
        } else {
            udp.broadcastTo("d", port);
        }
    } else if (20 < roll && pitch < -20) {
        drawDisplay("Left-Back");
        if (40 < roll || pitch < -40) {
            udp.broadcastTo("E", port);
        } else {
            udp.broadcastTo("e", port);
        }
    } else if (20 < roll && 20 < pitch) {
        drawDisplay("Right-Back");
        if (40 < roll || 40 < pitch) {
            udp.broadcastTo("F", port);
        } else {
            udp.broadcastTo("f", port);
        }
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
}


void loop() {
    M5.update();
    if (M5.BtnA.isReleased() && M5.BtnB.isReleased()) {
        readGyro();
    } else if (M5.BtnA.isPressed() && M5.BtnB.isReleased()) {
        drawDisplay("Button A");
        udp.broadcastTo("I", port);
    } else if (M5.BtnA.isReleased() && M5.BtnB.isPressed()) {
        drawDisplay("Button B");
        udp.broadcastTo("J", port);
    }
    delay(100);
}
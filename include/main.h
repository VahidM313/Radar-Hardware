#ifndef MAIN_H
#define MAIN_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include "driver/timer.h"
#include <SevSeg.h>
#include <NewPing.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ESP32Servo.h>

#define IrPin 35
#define servoPin 16 //rx2 pin
#define triggerPin 23
#define echoPin 22
#define maxDistance 450

void setupWifi();
void reconnect();
void publishData(int distance,int degree);
void calculateDistance();
void callback(char* topic, byte* payload, unsigned int length);
#endif

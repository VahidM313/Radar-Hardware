#ifndef MAIN_H
#define MAIN_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include "driver/timer.h"
#include <Servo.h>
#include <SevSeg.h>
#include <NewPing.h>
#include <WiFi.h>
#include <PubSubClient.h>

#define IrPin 35
#define servoPin 16
#define triggerPin 23
#define echoPin 22
#define maxDistance 450

#define TIMER_DIVIDER 16
#define TIMER_SCALE static_cast<int>(TIMER_BASE_CLK / TIMER_DIVIDER)
#define TIMER_INTERVAL_SEC (0.001)
#define TEST_WITH_RELOAD 1

void setupWifi();
void reconnect();
void publishData(int distance,int degree);
#endif

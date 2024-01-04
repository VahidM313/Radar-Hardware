#include "main.h"

const char* ssid = "VahidM313";
const char* password = "abc123abc";
const char* mqttServer = "broker.emqx.io";
constexpr int mqttPort = 1883;
const char* mqttTopic = "radar/data";

WiFiClient espClient;
PubSubClient client(espClient);

NewPing sonar(triggerPin, echoPin, maxDistance);
SevSeg sevseg;
Servo servo;
unsigned long MOVING_TIME = 3000;
unsigned long moveStartTime;

// void IRAM_ATTR timerGroup0Isr(void* arg)
// {
//     // Refresh the display
//     sevseg.refreshDisplay();
//
//     // Clear the interrupt
//     TIMERG0.int_clr_timers.t0 = 1;
//
//     // Re-enable the alarm
//     TIMERG0.hw_timer[TIMER_0].config.alarm_en = TIMER_ALARM_EN;
// }

void setup()
{
    Serial.begin(115200);
    pinMode(IrPin,INPUT);
    pinMode(servoPin,OUTPUT);
    servo.attach(servoPin);
    moveStartTime = millis();
    ///////////////////////////////// seven segment /////////////////////////////////
    // constexpr byte numDigits = 4;
    // constexpr byte digitPins[] = {0, 17, 4, 27}; // Replace with your actual GPIO pins
    // const byte segmentPins[] = {18, 21, 25, 33, 32, 19, 26}; // Replace with your actual GPIO pins
    // constexpr bool resistorsOnSegments = true; // 'false' means resistors are on digit pins
    // constexpr bool updateWithDelays = false; // Default. Recommended
    // constexpr bool leadingZeros = false; // Use 'true' if you'd like to keep the leading zeros
    // sevseg.begin(COMMON_ANODE, numDigits, digitPins,
    //              segmentPins, resistorsOnSegments, updateWithDelays, leadingZeros,
    //              true);
    // sevseg.setBrightness(90);
    /////////////////////////////////////////////////////////////////////////////////

    // constexpr timer_config_t config = {
    //     .alarm_en = TIMER_ALARM_EN,
    //     .counter_en = TIMER_PAUSE,
    //     .intr_type = TIMER_INTR_LEVEL,
    //     .counter_dir = TIMER_COUNT_UP,
    //     .auto_reload = TIMER_AUTORELOAD_EN,
    //     .divider = TIMER_DIVIDER,
    // };
    // timer_init(TIMER_GROUP_0, TIMER_0, &config);
    // timer_set_counter_value(TIMER_GROUP_0, TIMER_0, 0x00000000ULL);
    // timer_set_alarm_value(TIMER_GROUP_0, TIMER_0, TIMER_INTERVAL_SEC * TIMER_SCALE);
    // timer_enable_intr(TIMER_GROUP_0, TIMER_0);
    // timer_isr_register(TIMER_GROUP_0, TIMER_0, timerGroup0Isr, nullptr, ESP_INTR_FLAG_IRAM, nullptr);
    // timer_start(TIMER_GROUP_0, TIMER_0);

    setupWifi();
    client.setServer(mqttServer, 1883);
}

void loop()
{
    if (!client.connected())
        reconnect();
    // client.loop();
    int ultraDistance = 0;
    double irDistance = 0;
    for (int i = 0; i <= 10; i++)
    {
        ultraDistance += static_cast<int>(sonar.ping_cm());
        const auto volt = static_cast<float>(analogRead(IrPin)* (3.3 / 4095.0));
        Serial.println(volt);
        irDistance += 16.2537 * pow(volt,4) - 129.893 * pow(volt,3) + 382.268 * pow(volt,2) - 512.611 * volt + 301.439;
    }
    ultraDistance /= 10;
    irDistance /= 10;
    int average{};
    if (ultraDistance > 150 || ultraDistance < 20 || ultraDistance - irDistance >= 5)
        average = ultraDistance;
    else if (irDistance >= 20.0 && irDistance <= 150.0)
        average = static_cast<int>((ultraDistance + irDistance) / 2);
    Serial.printf("ir: %d, ultra: %d\n", static_cast<int>(irDistance), ultraDistance);
    // sevseg.setNumber(average);
    const int degree = random(0, 360);
    publishData(average,degree);
    delay(500);
}

void setupWifi() {
    delay(10);
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    while (WiFiClass::status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}

void reconnect() {
    while (!client.connected()) {
        Serial.print("Attempting MQTT connection...");
        if (client.connect("ESP8266Client")) {
            Serial.println("connected");
        } else {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
            delay(5000);
        }
    }
}

void publishData(const int distance,const int degree) {
    StaticJsonDocument<64> doc;  // Adjust the size based on your JSON structure
    doc["distance"] = distance;
    doc["degree"] = degree;
    char payload[64];
    serializeJson(doc, payload);
    client.publish(mqttTopic, payload);
}
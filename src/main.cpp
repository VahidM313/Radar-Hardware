#include "main.h"

const char *ssid = "VahidM313";
const char *password = "abc123abc";
const char *mqttServer = "192.168.108.251";
constexpr int mqttPort = 1883;
const char *mqttTopic = "radar/data";

WiFiClient espClient;
PubSubClient client(espClient);

NewPing sonar(triggerPin, echoPin, maxDistance);

Servo servo;

unsigned long moveStartTime;
int distance{};

int pos = 90;         // variable to store the servo position
int incomingByte = 0; // for incoming serial data

int degree{};
enum direction { stop, right, left };
direction dir{stop};

void setup() {
  Serial.begin(115200);
  pinMode(IrPin, INPUT);
  //   pinMode(servoPin, OUTPUT);
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);
  servo.setPeriodHertz(50); // standard 50 hz servo
  servo.attach(servoPin);
  servo.write(90);
  delay(2);
  // moveStartTime = millis();

  setupWifi();
  client.setServer(mqttServer, 1883);
}

void loop() {
  if (!client.connected())
      reconnect();
  client.loop();
  if (Serial.available() > 0) {
    // read the incoming byte:
    incomingByte = Serial.read();
    // say what you got:
    Serial.print("received: ");
    Serial.print(incomingByte);
    if (incomingByte == 108) {
      dir = left;
      Serial.println(" rotating left ");
      degree = 360;
    } else if (incomingByte == 114) {
      dir = right;
      Serial.println(" rotating right ");
      degree = 0;
    } else if (incomingByte == 115) {
      dir = stop;
      Serial.println(" Stopped ");
      servo.write(90);
    } else {
      Serial.println(" moving Random");
      servo.write(incomingByte);
    }
  }
  if (degree > 360 && dir == right) {
    dir = left;
    servo.write(90);
    delay(500);
  } else if (degree < 0 && dir == left) {
    dir = right;
    servo.write(90);
    delay(500);
  } else if (dir == stop) {
    servo.write(90);
    degree = 0;
  } else if (dir == right) {
    degree++;
    servo.write(85);
    // servo.write(80);
    delay(15);
  } else if (dir == left) {
    servo.write(99);
    // servo.write(105);
    delay(14);
    degree--;
  }
  calculateDistance();
  publishData(distance, degree);
  Serial.printf("degree: %d, distance: %d\n", degree, distance);
  // delay(500);
}

void calculateDistance() {
  int ultraDistance = 0;
  double irDistance = 0;
  for (int i = 0; i <= 10; i++) {
    ultraDistance += static_cast<int>(sonar.ping_cm());
    const auto volt = static_cast<float>(analogRead(IrPin) * (3.3 / 4095.0));
    irDistance += 16.2537 * pow(volt, 4) - 129.893 * pow(volt, 3) +
                  382.268 * pow(volt, 2) - 512.611 * volt + 301.439;
  }
  ultraDistance /= 10;
  irDistance /= 10;
  if (ultraDistance > 150 || ultraDistance < 20 ||
      ultraDistance - irDistance >= 5)
    distance = ultraDistance;
  else if (irDistance >= 20.0 && irDistance <= 150.0)
    distance = static_cast<int>((ultraDistance + irDistance) / 2);
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
    if (client.connect("ESP32Client")) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void publishData(const int distance, const int degree) {
  StaticJsonDocument<64> doc;
  doc["distance"] = distance;
  doc["degree"] = degree;
  char payload[64];
  serializeJson(doc, payload);
  client.publish(mqttTopic, payload);
}
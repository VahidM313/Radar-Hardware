#include "main.h"

const char *ssid = "VahidM313";
const char *password = "abc123abc";
const char *mqttServer = "172.18.221.233";
constexpr int mqttPort = 1883;
const char *mqttTopic = "radar/data";

WiFiClient espClient;
PubSubClient client(espClient);

NewPing sonar(triggerPin, echoPin, maxDistance);

Servo servo;

unsigned long moveStartTime;
int distance{};

const int rightMove{85}, leftMove{99}, brake{90};
int incomingByte = 0;

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
  servo.write(brake);
  delay(2);
  // moveStartTime = millis();

  setupWifi();
  client.setServer(mqttServer, 1883);
  client.setCallback(callback); 
}

void loop() {
  if (!client.connected())
    reconnect();
  client.loop();
  if (degree > 91 && dir == right) {
    dir = left;
    degree = 360;
    servo.write(brake);
    delay(500);
  } else if (degree < 280 && dir == left) {
    dir = right;
    degree = 0;
    servo.write(brake);
    delay(500);
  } else if (dir == stop) {
    servo.write(brake);
    if (degree)
      printf("\n %d degree \n", degree);
    degree = 0;
  } else if (dir == right) {
    degree++;
    servo.write(rightMove);
    delay(15);
  } else if (dir == left) {
    degree--;
    servo.write(leftMove);
    delay(14);
  }
  calculateDistance();
  if (dir == right) {
    publishData(distance, map(degree,0,91,0,360));

  } else if (dir == left) {
    publishData(distance, map(degree,360,280,360,0));
  }
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
      client.subscribe("radar/control");
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

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  String control = "";
  for (unsigned int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    control += (char)payload[i];
  }
  Serial.println();

  Serial.println(control);
  if (control == "stop") {
    dir = stop;
    Serial.println("stop callback");
  } else if (control == "left") {
    dir = left;
    Serial.println("left callback");
    degree = 360;
  } else if (control == "right") {
    dir = right;
    Serial.println("right callback");
    degree = 0;
  }
  printf("\ndir: %d\n",dir);
}

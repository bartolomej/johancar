#include <WiFi.h>

const char* wifiConfigs[1][2] = {
    {"A1-182E60", "T33NLTQTJL"}
};

const int WIFI_CONFIG_COUNT = sizeof(wifiConfigs) / sizeof(wifiConfigs[0]);
const int COMMAND_PORT = 8081;

const int LEFT_MOTOR_PIN = 12;
const int RIGHT_MOTOR_PIN = 14;

WiFiServer server(COMMAND_PORT);
WiFiClient client;

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("Starting ESP32...");

  pinMode(LEFT_MOTOR_PIN, OUTPUT);
  pinMode(RIGHT_MOTOR_PIN, OUTPUT);
  digitalWrite(LEFT_MOTOR_PIN, LOW);
  digitalWrite(RIGHT_MOTOR_PIN, LOW);
  Serial.println("Motor pins initialized");

  bool connected = false;
  for (int i = 0; i < WIFI_CONFIG_COUNT; i++) {
    Serial.printf("Attempting to connect to: %s\n", wifiConfigs[i][0]);
    WiFi.begin(wifiConfigs[i][0], wifiConfigs[i][1]);

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
      delay(500);
      Serial.print(".");
      attempts++;
    }

    if (WiFi.status() == WL_CONNECTED) {
      connected = true;
      Serial.println("\nWiFi connected!");
      Serial.printf("SSID: %s\n", wifiConfigs[i][0]);
      Serial.printf("IP address: %s\n", WiFi.localIP().toString().c_str());
      break;
    } else {
      Serial.printf("\nConnection failed %d, trying next network...", WiFi.status());
    }
  }

  if (!connected) {
    Serial.println("Failed to connect to any WiFi network!");
    return;
  }

  server.begin();
  Serial.printf("Command server started on port %d\n", COMMAND_PORT);
  Serial.println("Waiting for client connection...");
  
  // TODO: Instead of waiting for the Go server to connect (issue the first command) to the hardware, initiate a connection to the Go server first
  while (!client) {
    client = server.accept();
    delay(100);
  }
  
  Serial.println("Client connected!");
}

void loop() {
  if (!client || !client.connected()) {
    if (client) {
      client.stop();
      Serial.println("Client disconnected, waiting for new connection...");
    }
    client = server.available();
    if (client) {
      Serial.println("New client connected");
    }
    delay(100);
    return;
  }

  if (client.available()) {
    String command = client.readStringUntil('\n');
    command.trim();

    if (command.length() > 0) {
      Serial.printf("Received command: %s\n", command.c_str());

      int leftSpeed = 0;
      int rightSpeed = 0;

      int parsed = sscanf(command.c_str(), "%d,%d", &leftSpeed, &rightSpeed);
      
      if (parsed == 2) {
        Serial.printf("Parsed - left: %d, right: %d\n", leftSpeed, rightSpeed);
        controlMotors(leftSpeed, rightSpeed);
      } else {
        Serial.printf("Parse error: expected 2 values (left,right), got %d\n", parsed);
      }
    }
  }

  delay(10);
}

void controlMotors(int leftSpeed, int rightSpeed) {
  digitalWrite(LEFT_MOTOR_PIN, leftSpeed == 1 ? HIGH : LOW);
  digitalWrite(RIGHT_MOTOR_PIN, rightSpeed == 1 ? HIGH : LOW);
  
  Serial.printf("Motors: LEFT=%s, RIGHT=%s\n", 
                leftSpeed == 1 ? "ON" : "OFF",
                rightSpeed == 1 ? "ON" : "OFF");
}


#include <WiFi.h>

const char* wifiConfigs[1][2] = {
    {"A1-182E60", "T33NLTQTJL"}
};

const int WIFI_CONFIG_COUNT = sizeof(wifiConfigs) / sizeof(wifiConfigs[0]);
const int COMMAND_PORT = 8081;

const int LEFT_MOTOR_PIN = 12;
const int RIGHT_MOTOR_PIN = 14;
const float INTENSITY_THRESHOLD = 0.1;
const float ANGLE_THRESHOLD = 10.0;

WiFiServer server(COMMAND_PORT);

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
}

void loop() {
  WiFiClient client = server.available();

  if (client) {
    Serial.println("New client connected");

    while (client.connected()) {
      if (client.available()) {
        String command = client.readStringUntil('\n');
        command.trim();

        if (command.length() > 0) {
          Serial.printf("Received command: %s\n", command.c_str());

          float angle = 0.0;
          float intensity = 0.0;
          float x = 0.0;
          float y = 0.0;

          int parsed = sscanf(command.c_str(), "%f %f %f %f", &angle, &intensity, &x, &y);
          
          if (parsed == 4) {
            Serial.printf("Parsed - angle: %.2f, intensity: %.2f, x: %.2f, y: %.2f\n", 
                          angle, intensity, x, y);
            controlMotors(angle, intensity);
          } else {
            Serial.printf("Parse error: expected 4 values, got %d\n", parsed);
          }
        }
      }
    }

    client.stop();
    Serial.println("Client disconnected");
  }

  delay(10);
}

void controlMotors(float angle, float intensity) {
  if (intensity < INTENSITY_THRESHOLD) {
    digitalWrite(LEFT_MOTOR_PIN, LOW);
    digitalWrite(RIGHT_MOTOR_PIN, LOW);
    Serial.println("Motors: OFF (low intensity)");
    return;
  }

  if (angle < -ANGLE_THRESHOLD) {
    digitalWrite(LEFT_MOTOR_PIN, LOW);
    digitalWrite(RIGHT_MOTOR_PIN, HIGH);
    Serial.println("Motors: RIGHT ON (turning left)");
  } else if (angle > ANGLE_THRESHOLD) {
    digitalWrite(LEFT_MOTOR_PIN, HIGH);
    digitalWrite(RIGHT_MOTOR_PIN, LOW);
    Serial.println("Motors: LEFT ON (turning right)");
  } else {
    digitalWrite(LEFT_MOTOR_PIN, HIGH);
    digitalWrite(RIGHT_MOTOR_PIN, HIGH);
    Serial.println("Motors: BOTH ON (going straight)");
  }
}


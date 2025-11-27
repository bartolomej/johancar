#include <WiFi.h>

const char* wifiConfigs[][1] = {
  // {"YourHomeWiFi", "your_password"},
};

const int WIFI_CONFIG_COUNT = sizeof(wifiConfigs) / sizeof(wifiConfigs[0]);
const int COMMAND_PORT = 8081;

WiFiServer server(COMMAND_PORT);

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("Starting ESP32...");

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


#include <Wire.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "MTS4Z.h"

// I2C0 for MTS4Z (SDA=8, SCL=9)
TwoWire I2C_MTS4Z = TwoWire(0);
MTS4Z mts4z(I2C_MTS4Z);

// WiFi credentials
const char* ssid = "A_LITE UGANDA LTD";
const char* password = "@l!te@2025";

// Device API
const char* device_serial = "as-0515-99";
const char* device_base_url = "https://autothermo.neosaveuganda.com/api/device/";
const char* assignment_base_url = "https://autothermo.neosaveuganda.com/api/assignment/";

// Device state
String device_id = "";
String device_assignment_id = "";
String patient_id = "";
String bed_no = "";
String hospital_id = "";
bool assignmentSuccess = false;

// Smoothing parameters
float alpha = 0.2;
float smoothedTemp = 0.0;

// Calibration data
float thermometerReadings[] = {36.6, 36.7, 36.5, 36.6, 36.4};
float mts4zReadings[] = {14.54, 14.55, 14.56, 14.53, 14.52};

void setup() {
  Serial.begin(115200);
  delay(1000);

  I2C_MTS4Z.begin(8, 9, 100000);
  delay(1000);

  if (!mts4z.begin()) {
    Serial.println("❌ Failed to initialize MTS4Z!");
    while (1);
  }

  // Offset calibration
  float totalOffset = 0.0;
  int count = sizeof(thermometerReadings) / sizeof(thermometerReadings[0]);
  for (int i = 0; i < count; i++) {
    totalOffset += (thermometerReadings[i] - mts4zReadings[i]);
  }
  float averageOffset = totalOffset / count;
  mts4z.setTemperatureOffset(averageOffset);

  smoothedTemp = mts4z.readTemperature();
  Serial.println("✅ MTS4Z sensor initialized.");

  // Connect WiFi
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✅ Connected to WiFi.");

  checkDeviceStatus();
}

void loop() {
  float temp = mts4z.readTemperature();
  smoothedTemp = alpha * temp + (1 - alpha) * smoothedTemp;

  Serial.print("🌡️ Smoothed Temperature: ");
  Serial.print(smoothedTemp, 2);
  Serial.println(" °C");

  if (assignmentSuccess && device_assignment_id != "") {
    sendMonitoringData(smoothedTemp, 0, 0); // Only temperature; SpO2 & HR = 0
  } else {
    checkDeviceStatus();
  }

  delay(5000);
}

void checkDeviceStatus() {
  if ((WiFi.status() == WL_CONNECTED)) {
    HTTPClient http;
    String url = String(device_base_url) + device_serial;
    http.begin(url);
    int httpCode = http.GET();

    if (httpCode > 0) {
      String payload = http.getString();
      DynamicJsonDocument doc(1024);
      deserializeJson(doc, payload);

      if (doc.containsKey("devicestatus") && doc["devicestatus"].containsKey("device_id")) {
        device_id = doc["devicestatus"]["device_id"].as<String>();
        checkAssignment();
      } else {
        Serial.println("🚫 Invalid device ID in response.");
      }
    } else {
      Serial.println("🚫 Error fetching device status.");
    }
    http.end();
  }
}

void checkAssignment() {
  if ((WiFi.status() == WL_CONNECTED)) {
    HTTPClient http;
    String url = String(assignment_base_url) + device_id;
    http.begin(url);
    int httpCode = http.GET();

    if (httpCode > 0) {
      String payload = http.getString();
      DynamicJsonDocument doc(2048);
      deserializeJson(doc, payload);

      if (doc.containsKey("results")) {
        JsonObject result = doc["results"];
        device_assignment_id = result["device_assignment_id"].as<String>();
        patient_id = result["patient_id"].as<String>();
        bed_no = result["bed_no"].as<String>();
        hospital_id = result["hospital_id"].as<String>();

        assignmentSuccess = device_assignment_id != "" && device_assignment_id != "null";
      }
    } else {
      assignmentSuccess = false;
    }
    http.end();
  }
}

void sendMonitoringData(float temp, float spo2, float heart_rate) {
  if ((WiFi.status() == WL_CONNECTED) && patient_id != "" && hospital_id != "" && device_id != "") {
    HTTPClient http;
    String url = "https://autothermo.neosaveuganda.com/api/monitoring/";

    http.begin(url);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    String postData = "hospital_id=" + hospital_id +
                      "&patient_id=" + patient_id +
                      "&device_id=" + device_id +
                      "&spo2=" + String(spo2) +
                      "&heart_rate=" + String(heart_rate) +
                      "&temp=" + String(temp, 2);

    Serial.println("🌐 Sending data to server...");
    int httpResponseCode = http.POST(postData);

    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println("📡 Server response: " + response);
    } else {
      Serial.println("⚠️ Error sending data: " + String(httpResponseCode));
    }

    http.end();
  } else {
    Serial.println("🚫 Missing assignment or network info. Data not sent.");
  }
}

#include <WiFi.h>
#include <Wire.h>
#include <HTTPClient.h>
#define DHT_PIN 4
#define sensor_pin 33
const char *ssid = "Galaxy";
const char *password = "plyb7569";
const char *thingsboardServer = "demo.thingsboard.io";
const char *deviceToken = "XbGZV4mecdNQjkpLi1PS";
void setup() {
  Serial.begin(9600);
  Wire.begin();
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
}
void loop() {
  int humidity, temperature;
  int moisture_percentage;
  float moisture_analog;
  float moisture;
  if (readDHT11Data(DHT_PIN, &humidity, &temperature)) {
    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.print("%, Temperature: ");
    Serial.print(temperature);
Serial.println("°C"); }
  else {
    Serial.println("Failed to read data from DHT11 sensor");
}
  moisture = analogRead(sensor_pin);
  moisture_analog = moisture/4095;
  moisture_percentage = 100 - (100*moisture_analog);
  Serial.print("Soil Moisture(in Percentage) = ");
  Serial.print(moisture_percentage);
  Serial.println("%");
sendDataToThingsBoard(moisture_percentage, humidity, temperature);
  delay(2000);
}
bool readDHT11Data(int pin, int *humidity, int *temperature) {
  int data[5] = {0, 0, 0, 0, 0};
  pinMode(pin, OUTPUT);
  digitalWrite(pin, LOW);
  delay(18);
  digitalWrite(pin, HIGH);
  delayMicroseconds(40);
  pinMode(pin, INPUT);
  unsigned long timeout = micros() + 80;
  while (digitalRead(pin) == LOW) {
    if (micros() > timeout) {
      return false;
} }
  timeout = micros() + 80;
  while (digitalRead(pin) == HIGH) {
    if (micros() > timeout) {
      return false;
} }
  for (int i = 0; i < 40; i++) {
    timeout = micros() + 80;
    while (digitalRead(pin) == LOW) {
      if (micros() > timeout) {
        return false;
} }
    unsigned long startTime = micros();
    timeout = micros() + 80;
    while (digitalRead(pin) == HIGH) {
      if (micros() > timeout) {
        return false;
} }
    unsigned long duration = micros() - startTime;
    data[i / 8] <<= 1;
    if (duration > 40) {
      data[i / 8] |= 1;
    }
  }
  if (data[4] == ((data[0] + data[1] + data[2] + data[3]) & 0xFF)) {

*humidity = data[0];
    *temperature = data[2];
    return true;
}
  else {
    return false;
} }
void sendDataToThingsBoard(float moisture_percentage, int humidity, int temperature) {
  HTTPClient http;
  String url = "http://" + String(thingsboardServer) + "/api/v1/" +
  String(deviceToken) + "/telemetry";
  String payload = "{\"moisture\":" +
  String(moisture_percentage) + ", \"humidity\":" + String(humidity) + ",
  \"temperature\":" + String(temperature) + "}";
  http.begin(url);
  http.addHeader("Content-Type", "application/json");
  int httpResponseCode = http.POST(payload);
  if (httpResponseCode > 0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
}
else {
    Serial.print("HTTP POST request failed: ");
    Serial.println(httpResponseCode);
  }
  http.end();
}

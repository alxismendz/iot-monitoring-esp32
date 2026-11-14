/*
 * Monitoreo de temperatura y humedad con ESP32 (LilyGO TTGO T-Display) + DHT11
 * Muestra las lecturas en la pantalla TFT y las envía por HTTP POST (JSON)
 * a un endpoint de Node-RED, que las almacena en InfluxDB para Grafana.
 *
 * Credenciales y URL del servidor: copiar secrets.example.h -> secrets.h
 */

#include <WiFi.h>
#include <HTTPClient.h>
#include <TFT_eSPI.h>
#include <DHT.h>

#include "secrets.h"   // WIFI_SSID, WIFI_PASSWORD, SERVER_URL

// WiFi
const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;

// Endpoint de Node-RED (IP local de la PC donde corre Node-RED)
const char* serverName = SERVER_URL;

// DHT
#define DHTPIN 17
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// Pantalla
TFT_eSPI tft = TFT_eSPI();

void setup() {
  Serial.begin(115200);
  Serial.println("Iniciando ESP32...");

  // Conexión WiFi
  WiFi.begin(ssid, password);
  Serial.println("Conectando a WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nConectado a WiFi");
  Serial.println(WiFi.localIP());

  // Iniciar DHT y pantalla
  dht.begin();
  pinMode(4, OUTPUT);
  digitalWrite(4, HIGH); // backlight pantalla

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(3);
}

void loop() {
  delay(2000); // Espera 2 segundos

  float hum = dht.readHumidity();
  float temp = dht.readTemperature();

  if (isnan(hum) || isnan(temp)) {
    tft.fillRect(0, 0, 135, 240, TFT_BLACK);
    tft.drawString("Error DHT", 10, 50);
    return;
  }

  // Mostrar en pantalla
  tft.fillRect(0, 30, 135, 80, TFT_BLACK);
  tft.setCursor(10, 30);
  tft.print("Temp: ");
  tft.print(temp, 1);
  tft.print("C");

  tft.setCursor(10, 70);
  tft.print("Hum: ");
  tft.print(hum, 1);
  tft.print("%");

  // Enviar a Node-RED
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverName);
    http.addHeader("Content-Type", "application/json");

    String jsonData = "{\"temperatura\": ";
    jsonData += String(temp, 1);
    jsonData += ", \"humedad\": ";
    jsonData += String(hum, 1);
    jsonData += "}";

    int httpResponseCode = http.POST(jsonData);

    Serial.print("Código de respuesta HTTP: ");
    Serial.println(httpResponseCode);

    http.end();
  } else {
    Serial.println("WiFi no conectado");
  }
}

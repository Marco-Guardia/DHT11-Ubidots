#include <UbiConstants.h>
#include <UbiTypes.h>
#include <UbidotsEsp32Mqtt.h>

#include <TFT_eSPI.h>
#include <SPI.h>
#include <DHT.h>

//==================================================
// UBIDOTS
//==================================================

const char *UBIDOTS_TOKEN = "TU_TOKEN_AQUI";
const char *WIFI_SSID = "TU_SSID_AQUI";
const char *WIFI_PASS = "TU_PASSWORD_AQUI";

const char *DEVICE_LABEL = "Esp32";
const char *TEMPERATURE_LABEL = "Temperatura";
const char *HUMIDITY_LABEL = "Humedad";

Ubidots ubidots(UBIDOTS_TOKEN);

//==================================================
// DHT11
//==================================================

#define DHT_PIN 27
#define DHT_TYPE DHT11

DHT dht(DHT_PIN, DHT_TYPE);

//==================================================
// TFT
//==================================================

TFT_eSPI tft = TFT_eSPI();

#define TFT_BACKLIGHT 4

//==================================================
// TIEMPOS
//==================================================

const unsigned long INTERVALO_DHT = 2000;
const unsigned long INTERVALO_UBIDOTS = 5000;

unsigned long tiempoDHT = 0;
unsigned long tiempoUbidots = 0;

float temperatura = 0;
float humedad = 0;

//==================================================
// CALLBACK MQTT
//==================================================

void callback(char *topic, byte *payload, unsigned int length) {

  Serial.print("Mensaje recibido: ");

  for (unsigned int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }

  Serial.println();
}

//==================================================
// INTERFAZ TFT
//==================================================

void dibujarInterfaz() {

  tft.fillScreen(TFT_BLACK);

  tft.fillRect(0, 0, 240, 30, TFT_BLUE);

  tft.setTextColor(TFT_WHITE, TFT_BLUE);
  tft.setTextSize(2);
  tft.setCursor(35, 7);
  tft.print("DHT11 + UBIDOTS");

  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.setCursor(10, 45);
  tft.print("Temp:");

  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.setCursor(10, 82);
  tft.print("Hum:");
}

void mostrarDatos(float temp, float hum) {

  tft.fillRect(80, 38, 150, 70, TFT_BLACK);

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);

  tft.setCursor(90, 45);
  tft.print(temp, 1);

  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.print(" C");

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setCursor(90, 82);
  tft.print(hum, 0);

  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.print(" %");
}

void mostrarError() {

  tft.fillRect(80, 38, 150, 70, TFT_BLACK);

  tft.setTextColor(TFT_RED, TFT_BLACK);
  tft.setTextSize(2);

  tft.setCursor(90, 55);
  tft.print("ERROR");
}

//==================================================
// SETUP
//==================================================

void setup() {

  Serial.begin(115200);

  pinMode(TFT_BACKLIGHT, OUTPUT);
  digitalWrite(TFT_BACKLIGHT, HIGH);

  tft.init();
  tft.setRotation(1);

  dibujarInterfaz();

  dht.begin();

  ubidots.connectToWifi(WIFI_SSID, WIFI_PASS);
  ubidots.setCallback(callback);

  ubidots.setup();
  ubidots.reconnect();

  tiempoDHT = millis();
  tiempoUbidots = millis();
}

//==================================================
// LOOP
//==================================================

void loop() {

  if (!ubidots.connected()) {
    ubidots.reconnect();
  }

  ubidots.loop();

  // Lectura DHT cada 2 segundos

  if (millis() - tiempoDHT >= INTERVALO_DHT) {

    tiempoDHT = millis();

    humedad = dht.readHumidity();
    temperatura = dht.readTemperature();

    if (isnan(humedad) || isnan(temperatura)) {

      Serial.println("Error DHT11");
      mostrarError();
      return;
    }

    Serial.print("Temperatura: ");
    Serial.print(temperatura);

    Serial.print(" C  Humedad: ");
    Serial.print(humedad);

    Serial.println(" %");

    mostrarDatos(temperatura, humedad);
  }

  // Enviar a Ubidots cada 5 segundos

  if (millis() - tiempoUbidots >= INTERVALO_UBIDOTS) {

    tiempoUbidots = millis();

    ubidots.add(TEMPERATURE_LABEL, temperatura);
    ubidots.add(HUMIDITY_LABEL, humedad);

    ubidots.publish(DEVICE_LABEL);

    Serial.println("Datos enviados a Ubidots");
  }
}

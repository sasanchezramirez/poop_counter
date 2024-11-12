#include <WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <HTTPClient.h>

char ssid[] = "S24 Ultra de Santiago"; // Reemplaza con tu SSID
char pass[] = "leomessi10"; // Reemplaza con tu contraseña de WiFi

#define TRIG_PIN 17 
#define ECHO_PIN 16
#define LED_PIN 2
#define CLEAN_LED_PIN 26

const unsigned long timeout = 3000000UL; // Tiempo de espera para el sensor
unsigned int ledActivations = 0; // Contador de activaciones del LED
unsigned int activationsUntilCleaning = 0; // Contador de activaciones hasta la limpieza
unsigned long previousMillis = 0; // Tiempo anterior para el muestreo
const long sampleInterval = 5000; // Intervalo de muestreo en milisegundos (5 segundos)
bool chaplinIsPresent = false; // Estado de si Chaplin está presente
bool chaplinWasPresent = false; // Estado anterior de si Chaplin estaba presente

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", -18000, 60000); // GMT -5 para Bogotá, actualización cada 60 segundos

void setup() {
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(CLEAN_LED_PIN, OUTPUT);
  Serial.begin(115200); // ESP32 suele funcionar bien con una velocidad mayor

  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");

  timeClient.begin();
}

void loop() {
  timeClient.update();

  int currentHour = timeClient.getHours();
  int currentMinute = timeClient.getMinutes();

  if (currentHour == 23 && currentMinute == 59) {
    ledActivations = 0;
    activationsUntilCleaning = 0;
  }

  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= sampleInterval) {
    previousMillis = currentMillis;

    // Medir la distancia con el sensor ultrasónico
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);
  
    long duration = pulseIn(ECHO_PIN, HIGH, timeout);
    int distance = duration * 0.034 / 2;

    // Determinar si Chaplin está presente o no
    if (distance > 0 && distance < 24) {
      chaplinIsPresent = true;
    } else {
      chaplinIsPresent = false;
    }

    // Detectar cambios en el estado de presencia de Chaplin
    if (chaplinIsPresent && !chaplinWasPresent) {
      // Chaplin acaba de entrar
      ledActivations++;
      activationsUntilCleaning++;
      String message = "Chaplin está en el baño, ya van " + String(ledActivations) + " veces que ha ido al baño el día de hoy";
      Serial.println(message);

      // Enviar datos al backend
      if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        http.begin("https://7rcszg76-8000.brs.devtunnels.ms/poop/new-poop"); // Reemplaza con la URL de tu servidor
        http.addHeader("Content-Type", "application/json");
        String httpRequestData = "{\"pet_id\": 1}";
        int httpResponseCode = http.POST(httpRequestData);

        if (httpResponseCode > 0) {
          String response = http.getString();
          Serial.println(httpResponseCode);
          Serial.println(response);
        } else {
          Serial.print("Error en el envío: ");
          Serial.println(httpResponseCode);
        }
        http.end();
      }

      digitalWrite(LED_PIN, HIGH);
    } else if (chaplinIsPresent && chaplinWasPresent) {
      // Chaplin sigue en el baño
      Serial.println("Chaplin aún está en el baño.");
    } else if (!chaplinIsPresent && chaplinWasPresent) {
      // Chaplin acaba de salir
      Serial.println("Chaplin ha salido del baño.");
      digitalWrite(LED_PIN, LOW);
    } else {
      // Chaplin no está presente
      Serial.println("Preparado para recibir a Chaplin.");
    }

    // Actualizar el estado anterior
    chaplinWasPresent = chaplinIsPresent;

    // Control del LED de limpieza
    if (activationsUntilCleaning >= 7){
      digitalWrite(CLEAN_LED_PIN, HIGH); // Enciende el LED de limpieza
    } else {
      digitalWrite(CLEAN_LED_PIN, LOW); // Apaga el LED de limpieza
    }
  }
}

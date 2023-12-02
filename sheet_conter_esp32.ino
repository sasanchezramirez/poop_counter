#include <WiFi.h>

#define BLYNK_TEMPLATE_ID "TMPL2f4gzsWW6"
#define BLYNK_TEMPLATE_NAME "Quickstart Device"
#define BLYNK_AUTH_TOKEN "SXcqI69IplTVrkhsw2106zkIF6OBu7Wn"

#include <BlynkSimpleEsp32.h>

char ssid[] = "CERN";
char pass[] = "Chepefortuna42";

#define TRIG_PIN 17 
#define ECHO_PIN 16
#define LED_PIN 2

const unsigned long timeout = 3000000UL; // Por ejemplo, 1 segundo
unsigned int ledActivations = 0; // Contador de activaciones del LED
unsigned long previousMillis = 0; // Tiempo anterior para el muestreo
const long sampleInterval = 5000; // Intervalo de muestreo en milisegundos (5 segundos)
bool ledWasOn = false; // Estado anterior del LED

void setup() {
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  Serial.begin(115200); // ESP32 suele funcionar bien con una velocidad mayor

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass); // Cambio aquí

}

void loop() {
  
  Blynk.run();

  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= sampleInterval) {
    previousMillis = currentMillis;

    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);
  
    long duration = pulseIn(ECHO_PIN, HIGH);
    int distance = duration * 0.034 / 2;

    if (distance < 24) {
      if (!ledWasOn) {
        ledActivations++;
        String message = "Chaplin está en el baño, ya van " + String(ledActivations) + " veces que ha ido al baño el día de hoy";
        Serial.println(message);
        Blynk.virtualWrite(V4, "Chaplin está en el baño");
        Blynk.virtualWrite(V0, ledActivations);
        ledWasOn = true;
      }
      digitalWrite(LED_PIN, HIGH);
    } else {
      ledWasOn = false;
      digitalWrite(LED_PIN, LOW);
    }
  }
}

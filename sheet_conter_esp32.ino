#include <WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>

#define BLYNK_TEMPLATE_ID "TMPL27MS3jDXE"
#define BLYNK_TEMPLATE_NAME "ChaplinPoopManager"
#define BLYNK_AUTH_TOKEN "pAGCGQ6EHvqQZP_rerp0cBfCF14cgy4F"

#include <BlynkSimpleEsp32.h>

char ssid[] = "CERN";
char pass[] = "Chepefortuna42";

#define TRIG_PIN 17 
#define ECHO_PIN 16
#define LED_PIN 2
#define CLEAN_LED_PIN 26

const unsigned long timeout = 3000000UL; // Por ejemplo, 1 segundo
unsigned int ledActivations = 0; // Contador de activaciones del LED
unsigned long previousMillis = 0; // Tiempo anterior para el muestreo
const long sampleInterval = 5000; // Intervalo de muestreo en milisegundos (5 segundos)
bool ledWasOn = false; // Estado anterior del LED

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", -18000, 60000); // GMT -5 para Bogotá, actualización cada 60 segundos

BLYNK_WRITE(V2) { // Asegúrate de que V2 sea el pin virtual asignado a tu botón en la app Blynk
    int buttonState = param.asInt(); // Obtiene el estado del botón (1 = presionado, 0 = no presionado)

    if (buttonState == 1) {
        ledActivations = 0; // Resetea el contador
        Blynk.virtualWrite(V0, ledActivations); // Actualiza el valor en Blynk
        Serial.println("ledActivations reseteado a 0");
    }
}
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
  Serial.println("WiFi connected");

  timeClient.begin();

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass); // Cambio aquí
}

void loop() {
  Blynk.run();
  timeClient.update();

  int currentHour = timeClient.getHours();
  int currentMinute = timeClient.getMinutes();

  if (currentHour == 23 && currentMinute == 59) {
    ledActivations = 0;
  }

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

    if (ledActivations >= 7){
      digitalWrite(CLEAN_LED_PIN, HIGH); // Enciende el nuevo LED
      Blynk.virtualWrite(V1, "Debes limpiar la arena");
    } else {
      digitalWrite(CLEAN_LED_PIN, LOW); // Asegúrate de apagar el LED cuando la condición ya no se cumpla
    }


  }
}

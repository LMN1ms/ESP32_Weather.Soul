//dichiarazione librerie
#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP085.h>
#include <DHT.h>

//connessione alla rete wifi
const char *ssid = "IoT_EXT";
const char *password = "CasaIoT2024|";

//connessione alla pagina dati di thingspeak
const char *apiKey = "X6GREUFAZIYLU48U";

//pin collegati al sensore BMP180
const int bmp180SDA = 21;  // SDA
const int bmp180SCL = 22;  // SCL
//gestione sensore pressione
Adafruit_BMP085 bmp;

//pin collegato al sensore DHT22
#define DHTPIN 4
//gestione sensore temperatura e umidità
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

//pin collegato al sensore del terreno
#define AOUT_PIN 34


//setup
void setup() {
  Serial.begin(115200);
  
  //connessione al Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    Serial.print("NON CONNESSO ");
  }
  Serial.println("");
  Serial.println("CONNESSO AL WIFI");

  //inizializza il sensore BMP180
  if (!bmp.begin()) {
    Serial.println("SENSORE PRESSIONE NON TROVATO!");
    while (1);
  }

  //inizializza il sensore DHT22
  dht.begin();
}

void loop() {

  //lettura dati dal sensore DHT22
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();
  
  //lettura dati dal sensore BMP180
  float pressure = bmp.readPressure() / 100.0F;
  float temperaturebmp = bmp.readTemperature();

  //lettura sensore terreno
  int value = analogRead(AOUT_PIN);
  

  //stampa i dati dei sensori
  Serial.print("Temperatura: ");
  Serial.print(temperature);
  Serial.println(" °C");
  Serial.print("Umidita: ");
  Serial.print(humidity);
  Serial.println(" %");
  Serial.print("Pressione: ");
  Serial.print(pressure);
  Serial.println(" hPa");
  Serial.print("TemperaturaBMP: ");
  Serial.print(temperaturebmp);
  Serial.print("Umidita terreno: ");
  Serial.print(value);
  int moisturePercentage = map(value, 2650, 1050, 0, 100);
  Serial.println(moisturePercentage);
  Serial.println(" %");
  
  //invia dati a ThingSpeak
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClient client;

    //prepara l'URL API
    String url = "/update?api_key=";
    url += apiKey;
    url += "&field1=";
    url += String(temperature);
    url += "&field2=";
    url += String(humidity);
    url += "&field3=";
    url += String(pressure);
    url += "&field4=";
    url += String(temperaturebmp);
    url += "&field5=";
    url += String(moisturePercentage);

    //effettua la richiesta HTTP
    if (client.connect("api.thingspeak.com", 80)) {
      Serial.println("Connected to ThingSpeak");

      client.print("GET " + url + " HTTP/1.1\r\n" +
                   "Host: api.thingspeak.com\r\n" +
                   "Connection: close\r\n\r\n");
      delay(500);

      //leggi e stampa la risposta
      while (client.available()) {
        String line = client.readStringUntil('\r');
        Serial.print(line);
      }

      Serial.println();
      Serial.println("Dati inviati a thingspeak");

      client.stop();
    } else {
      Serial.println("Connessione a thingspeak fallita");
    }
  }


  //attesa di 120 secondi prima di inviare nuovamente i dati
  delay(120000);
}

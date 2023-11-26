#include <DHT.h>
#include <WiFi.h>
#include "freertos/FreeRTOS.h"

String apiKey = "QSR26T80SOHFOI13";

const char *ssid = "VIVOFIBRA-8D50";
const char *pass = "184E53AF32";
const char* server = "api.thingspeak.com";
// const char *server = "184.106.153.149";

#define DHTPIN 4  //pin where the dht11 is connected

DHT dht(DHTPIN, DHT11);

WiFiClient client;

void sendTemperature(void *param){
  float t = dht.readTemperature();

  if (isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  if (client.connect(server, 80))
  {

    String postStr = apiKey;
    postStr += "&field1=";
    postStr += String(t);
    postStr += "\r\n\r\n";

    client.print("POST /update HTTP/1.1\n");
    client.print("Host: api.thingspeak.com\n");
    client.print("Connection: close\n");
    client.print("X-THINGSPEAKAPIKEY: " + apiKey + "\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(postStr.length());
    client.print("\n\n");
    client.print(postStr);
  }
  client.stop();

  Serial.print("Temperature: ");
  Serial.println(t);

  // thingspeak needs minimum 15 sec delay between updates
  delay(1000);
}

void sendHumidity(void *param){
  float h = dht.readHumidity();

  if (isnan(h)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  if (client.connect(server, 80))
  {

    String postStr = apiKey;
    postStr += "&field2=";
    postStr += String(h);
    postStr += "\r\n\r\n";

    client.print("POST /update HTTP/1.1\n");
    client.print("Host: api.thingspeak.com\n");
    client.print("Connection: close\n");
    client.print("X-THINGSPEAKAPIKEY: " + apiKey + "\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(postStr.length());
    client.print("\n\n");
    client.print(postStr);
  }
  client.stop();

  Serial.print(" degrees Celcius, Humidity: ");
  Serial.println(h);

  // thingspeak needs minimum 15 sec delay between updates
  delay(1000);
}

void setup() {
  Serial.begin(115200);
  delay(10);
  dht.begin();

  Serial.println("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  xTaskCreate(sendTemperature,"Temperature",2048,NULL,1,NULL);
  xTaskCreate(sendHumidity,"Humidity",2048,NULL,1,NULL);
}

void loop() {

}

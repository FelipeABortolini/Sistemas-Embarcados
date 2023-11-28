#include <DHT.h>
#include <WiFi.h>
#include "ThingSpeak.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/timers.h>

#define CHANNEL_ID 2358437
#define CHANNEL_API_KEY "QSR26T80SOHFOI13"

#define DHTPIN 4

const char *ssid = "VIVOFIBRA-8D50";
const char *pass = "184E53AF32";

DHT dht(DHTPIN, DHT11);

WiFiClient client;

// Timer para leitura periódica do sensor
TimerHandle_t timerDHTRead;

/* Descritor de task */
TaskHandle_t taskSend;

// Função para ler dados do sensor DHT
void readDHTSensor(TimerHandle_t xTimer) {
  // Notifica a tarefa para iniciar a leitura do sensor
  xTaskNotifyGive(taskSend);
}

// Função para enviar dados para o ThingSpeak
void sendDataToThingSpeak(float temperature, float humidity) {
  ThingSpeak.writeField(CHANNEL_ID, 1, temperature, CHANNEL_API_KEY);
  ThingSpeak.writeField(CHANNEL_ID, 2, humidity, CHANNEL_API_KEY);
  ThingSpeak.writeField(CHANNEL_ID, 3, 1, CHANNEL_API_KEY);

  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.print(" degrees Celsius, Humidity: ");
  Serial.print(humidity);
  Serial.println("%. Sent to ThingSpeak.");

  Serial.println("Waiting...");
}

// Função da tarefa de envio para o ThingSpeak
void taskSendToThingSpeak(void *pvParameters) {
  float temperature, humidity;

  while (1) {
    // Aguarda notificação para iniciar a leitura do sensor
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    // Realiza a leitura do sensor DHT
    temperature = dht.readTemperature();
    humidity = dht.readHumidity();

    // Envia os dados para o ThingSpeak
    sendDataToThingSpeak(temperature, humidity);
  }
}

// Função da tarefa de gerenciamento de WiFi
void taskWiFi(void *pvParameters) {
  WiFi.begin(ssid, pass);

  while (1) {
    // Verifica o status da conexão WiFi a cada 10 segundos
    vTaskDelay(10000 / portTICK_PERIOD_MS);

    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("Conexão WiFi perdida. Reconectando...");

      WiFi.begin(ssid, pass);

      int attempts = 0;
      while (WiFi.status() != WL_CONNECTED && attempts < 10) {
        delay(1000);
        attempts++;
      }

      if (WiFi.status() == WL_CONNECTED) {
        Serial.println("Conectado à rede WiFi!");
      } else {
        Serial.println("Falha na reconexão WiFi. Verifique as credenciais.");
      }
    }
  }
}

void setup() {
  Serial.begin(115200);

  delay(10);
  dht.begin();
  ThingSpeak.begin(client);

  // Inicializa o timer para leitura periódica do sensor DHT
  timerDHTRead = xTimerCreate("timerDHTRead", pdMS_TO_TICKS(15000), pdTRUE, (void *)0, readDHTSensor);
  xTimerStart(timerDHTRead, 0);

  // Inicia as tarefas
  xTaskCreatePinnedToCore(taskWiFi, "WiFiTask", 4096, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(taskSendToThingSpeak, "ThingSpeakTask", 4096, NULL, 1, &taskSend, 1);
}

void loop() {
  
}
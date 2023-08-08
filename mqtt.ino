#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DHT11.h>

////////////////////////////////////////////////////////////////////////////
// Pinos e configurações: modificar para corresponder à sua configuração
////////////////////////////////////////////////////////////////////////////

#define MQTT_CLIENT_ID "NodeMCU-Client_1"
#define MQTT_USER "usuarioMQTT"
#define MQTT_PASS "senhaMQTT"
#define MQTT_SERVER "IP do MQTT broker"
#define MQTT_PORT 1883

#define WIFI_SSID "SSID da WIFI"
#define WIFI_PASS "senhaWIFI"

#define LED D1
#define HUMIDITY_SENSOR D2

////////////////////////////////////////////////////////////////////////////
// Variáveis globais
////////////////////////////////////////////////////////////////////////////

WiFiClient wlanclient;
PubSubClient mqttClient(wlanclient);

// Sensor de umidade
DHT11 dht11(HUMIDITY_SENSOR);
float UltimoValor = 0;
int TimeCounter = 0;

////////////////////////////////////////////////////////////////////////////
// WIFI
////////////////////////////////////////////////////////////////////////////

void connectWIFI()
{
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  Serial.println("Connecting to Wifi");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  Serial.print("\nConnected to WiFi, Got an IP address :");
  Serial.println(WiFi.localIP());
}

////////////////////////////////////////////////////////////////////////////
// MQTT (Conexão e callback)
////////////////////////////////////////////////////////////////////////////

void reconnectMQTT()
{
  while (!mqttClient.connected())
  {
    Serial.println("Conectando ao Broker MQTT");
    mqttClient.connect(MQTT_CLIENT_ID, MQTT_USER, MQTT_PASS);
    delay(1000);
    Serial.println(mqttClient.state());
    delay(2000);
  }
  Serial.println("MQTT conectado");
}

// Callback para ligar e desligar o LED
void mqttCallback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Message arrived on Topic:");
  Serial.println(topic);

  payload[length] = '\0';
  String message((char *)payload);

  Serial.print("Message: ");
  Serial.println(message);
  if (message == "on")
  {
    digitalWrite(LED, HIGH);
  }
  if (message == "off")
  {
    digitalWrite(LED, LOW);
  }
}

///////////////////////////////////////////////////////////////////////
// Setup: inicializa conexões e configura o sensor e o LED
///////////////////////////////////////////////////////////////////////

void setup()
{
  // Conexão serial: alterar dependendo da placa
  Serial.begin(9600);

  // Definir pino do LED como output
  pinMode(LED, OUTPUT);

  // Conectar ao WIFI
  connectWIFI();

  // Conectar ao broker de MQTT
  mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
  mqttClient.setCallback(mqttCallback);
  if (!mqttClient.connected())
  {
    reconnectMQTT();
  }

  // Configura o sensor através do MQTT Discovery do Home Assistant
  String novosensor = "{\"expire_after\": \"600\", \"unique_id\":\"umidade_nodemcu_1\", \"device_class\":\"humidity\",\"unit_of_measurement\":\"%\", \"name\": \"Umidade NodeMCU 1\", \"state_topic\": \"nodemcu/umidade/state\"}";
  mqttClient.publish("homeassistant/sensor/nodemcu/umidade/config", novosensor.c_str(), false);
  delay(500);

  // Cria um subscriber para receber comandos do Home Assistant
  mqttClient.subscribe("nodemcu/led/command");
}

///////////////////////////////////////////////////////////////////////
// Loop: Logica principal do codigo
///////////////////////////////////////////////////////////////////////

void loop()
{
  // Reconecta ao broker de MQTT se a conexão caiu
  if (!mqttClient.connected())
  {
    reconnectMQTT();
  }

  // Recebe os eventos do subscriber e ativa o callback se há um novo evento
  mqttClient.loop();

  // Leitura do sensor de umidade
  float humidity = dht11.readHumidity();
  String humidity_str((float)humidity);

  // Verifica se houve erro na leitura
  if (humidity != -1)
  {
    // Imprime umidade no Serial
    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.println(" %");

    // Publica a umidade via MQTT se o valor mudou
    if (humidity != UltimoValor)
    {
      UltimoValor = humidity;
      mqttClient.publish("nodemcu/umidade/state", humidity_str.c_str());
      TimeCounter = 0;
    }

    // Publica a umidade via MQTT se tem mais de 10 minutos desde a ultima atualização
    if (TimeCounter < 300)
    {
      TimeCounter++;
    }
    else if (TimeCounter >= 300)
    {
      mqttClient.publish("nodemcu/umidade/state", humidity_str.c_str());
      TimeCounter = 0;
    }
  }
  else
  {
    // Mensagem de erro em caso de falha na leitura do sensor de umidade
    Serial.println("Error reading humidity");
  }
  // 2 segundos de delay entre iterações
  delay(2000);
}
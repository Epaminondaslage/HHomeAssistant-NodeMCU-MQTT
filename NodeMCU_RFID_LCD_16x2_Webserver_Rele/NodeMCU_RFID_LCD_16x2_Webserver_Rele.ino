
/*
   ----------------------------------------------------------------------------
   Sistema de controle de acesso
   Placa utilizada NodeMCU
   Este Skech utiliza o shild MFRC522 library
   Display
   Adaptado de :
   https://jualabs.wordpress.com/2016/09/26/controle-de-acesso-com-nodemcu-rfid/
   https://jualabs.wordpress.com/2015/07/23/controle-de-acesso-com-arduinorfid/
   Implementações futuras
    + Falta salvar os cartoes em eprom para poder liberar sem a rede funcionando
    + Cada pessoa terá dois cartões para usar em duas portas
   Ver este tutorial
   https://www.instructables.com/id/Wireless-RFID-Door-Lock-Using-Nodemcu/
   ----------------------------------------------------------------------------
                   Correspondencia de pinos entre módulo e placas
   -----------------------------------------------------------------
               MFRC522      NodeMCU
   Signal      Pin          Pin      GPIO
   -----------------------------------------------------------------
   RST/Reset   RST           D3    GPIO 00
   SPI SS      SDA(SS)       D4    GPIO 02
   SPI MOSI    MOSI          D7    GPIO 13
   SPI MISO    MISO  SCL     D6    GPIO 12
   SPI SCK     SCK           D5    GPIO 14
   Power       VCC           3.3V
   GND         GND           GND
  -----------------------------------------------------------------
               LCD 16x2 I2C NodeMCU
   Signal      Pin          Pin      GPIO
   -----------------------------------------------------------------
    GND         GND          GND
    VCC         VCC          VIN
    SCL         SCL          D1       GPIO 05
    SDA         SDA          D2       GPIO 04
   -----------------------------------------------------------------
    Módulo Relé duplo - 3.3V
    Signal      Pin          Pin      GPIO
   -----------------------------------------------------------------
    GND         GND          GND
    VCC         VCC          3.3V
    In1                       D0
    In2                       D8
  -----------------------------------------------------------------
  Dados do servidor e Banco de dados
  -----------------------------------------------------------------

  -----------------------------------------------------------------
  Dados do Mqtt
  -----------------------------------------------------------------



  -----------------------------------------------------------------*/


// Pinos do RFI MRFC 522
#define RST_PIN  D3 // RST    -PIN für RC522 - RFID  GPIO 00 
#define SS_PIN  D4  // SDA(SS)-PIN für RC522 - RFID  GPIO 02 

//pino de saida para acionamento do relé
#define P1 10    // pin SD3 gpio 10
#define P2 9    // pin D8 gpio 9

//https://github.com/esp8266/Arduino/tree/master/libraries/ESP8266WebServer
#include <ESP8266WebServer.h>

//https://github.com/esp8266/Arduino/tree/master/libraries/ESP8266mDNS
#include <ESP8266mDNS.h>


//Biblioteca wifi do nodeMCUESP8266
#include <ESP8266WiFi.h>

//https://www.arduino.cc/en/Reference/SPI
#include <SPI.h>

//Biblioteca do RFID
//https://github.com/miguelbalboa/rfid
#include <MFRC522.h>

// Biblioteca que permite comunicar I2C / TWI dispositivos
//https://www.arduino.cc/en/Reference/Wire
#include <Wire.h>

//Biblioteca do display LCD
//https://github.com/fdebrabander/Arduino-LiquidCrystal-I2C-library
#include <LiquidCrystal_I2C.h>

/* Programa original usa a lib abaixo
   INCLUSÃO DE BIBLIOTECAs  Display LCD I2C
  //https://gitlab.com/tandembyte/liquidcrystal_i2c
  #include <LiquidCrystal_I2C.h>*/

//Biblioteca do clientMQTT
//http://pubsubclient.knolleary.net/api.html
//https://github.com/knolleary/pubsubclient
#include <PubSubClient.h>

//Criando WIFIClient
WiFiClient espClient;

//Criando o clientMQTT com o wificlient
PubSubClient client(espClient);

//Criando LCD com posições dos pinos,tamanho da linha, tamanho coluna.
LiquidCrystal_I2C lcd(0x27, 16, 2);

//Criando RFID nas posições dos pinos.
MFRC522 mfrc522(SS_PIN, RST_PIN);

//WIFI parametros.
const char* ssid = "123644987";
const char* password = "planeta514";

// Inicializa Servidor Web
ESP8266WebServer server(80);

//informações do broker MQTT - Verifique as informações geradas pelo CloudMQTT
const char* mqtt_server = "postman.cloudmqtt.com";
const int mqtt_port = 14190;
const char* mqttUser = "vovrrdtu";                 //user
const char* mqttPassword = "6eFMkVHfNnMX";        //password
const char* mqttTopicSub = "pedeserra";         //tópico que sera assinado/

//  Send HTTP status 200 (Ok) and send some text to the browser/client
void handleRoot() {
  server.send(200, "html", homePage());
}

// Send HTTP status 404 (Not Found) when there's no handler for the URI in the request
void handleNotFound() {
  String message = "Arquivo nao encontrado \n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "Página Não Encontrada ", message);
}


void setup() {
  //Definindo porta de saida do Serial
  Serial.begin(9600);

  // Define pino de saída
  pinMode(P1, OUTPUT);
  pinMode(P2, OUTPUT);

  digitalWrite(P1, HIGH);          //coloca saída em LOW  para desligar o P1
  digitalWrite(P2, HIGH);          //coloca saída em LOW  para desligar o P2


  //Chamando método de conexão WIFI
  setup_wifi();

  //Definindo server mqtt do Client
  client.setServer(mqtt_server, mqtt_port);

  //Definindo método callback que irá receber os callbacks do client criado.
  client.setCallback(callback);

  // Init SPI bus
  SPI.begin();

  //Iniciando PCD do RFID
  mfrc522.PCD_Init();

  // GPIO utilizada por dispositivos I2C
  //SCL         SCL          D1       GPIO 05
  //SDA         SDA          D2       GPIO 04
  Wire.begin(4, 5);

  //Iniciando LCD
  lcd.init();
  lcd.backlight(); // HABILITA O BACKLIGHT (LUZ DE FUNDO)
  lcd.setCursor(0, 1); //SETA A POSIÇÃO EM QUE O CURSOR RECEBE O TEXTO A SER MOSTRADO(LINHA 2)
  lcd.print(" Aproxime o TAG"); //ESCREVE O TEXTO NA SEGUNDA LINHA DO DISPLAY LCD
  // printLCD("Passe o Cartao!");
}

//Método de conexão com rede WIFI
void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Conectando com ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");

  //Importante : Configura NodeMCU com IP fixo. Ajustar para sua rede
  //IPAddress subnet(255, 255, 255, 0);
  //WiFi.config(IPAddress(10, 0, 0, 26), IPAddress(10, 0, 0, 1), subnet);
  Serial.println("IP Fixo");

  Serial.println("WiFi conectado");
  Serial.println("Endereco IP : ");
  Serial.println(WiFi.localIP());
  delay(200);
  if (MDNS.begin("esp8266")) {
    Serial.println("MDNS Inicializado");
  }

  server.on("/", handleRoot);

  server.on("/inline", []() {
    server.send(200, "text/plain", "Esta funcinando bem");
  });

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server inicializado");

}

//Método de conexão com servidor MQTT
void conectMqtt() {
  while (!client.connected()) {
    Serial.print("Conectando ao MQTT ...");

    //Parametros são nodeMCUClient, usuárioMQTT, senhaMQTT
    if (client.connect(mqttUser, mqttUser, mqttPassword)) {
      Serial.println("Conectado");
      //Inscrevendo-se no tópico retorno.
      client.subscribe("retorno");
    } else {
      Serial.print("Falha, rc=");
      Serial.print(client.state());
      Serial.println(" Tentando novamente em 5 segundos");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

//Método que foi definido para receber os retornos dos tópicos que demos subscribe,
// neste caso apenas o tópico 'retorno'
//Parametros: NomedoTópico, mensagem , tamanho da mensagem
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.println();
  Serial.print("Messagem recebida [");
  Serial.print(topic);
  Serial.print("]: ");
  String mensagem = "";
  //Conversão da mensagem recebidade de byte pra String
  for (int i = 0; i < length; i++) {
    mensagem += (char)payload[i];
  }
  Serial.println(mensagem);
  Serial.println();

  //Chamada ao método que controla o acesso
  verificaAcesso(mensagem);
}

//Método de controle de acesso.
void verificaAcesso(String mensagem) {
  // Estrutura da mensagem recebida
  // ENTRANDO/Nome_do_dono_do_cartao
  // SAINDO/Nome_do_dono_do_cartao
  // Retorno FALSE é dado sempre que o id do cartão não existe no servidor
  // ou ocorreu alguma falha de validação
  // FALSE

  if ( mensagem.substring(0, 8) == "ENTRANDO" ) {
    printLCD("BEM VINDO");
    delay(1000);
    printLCD(mensagem.substring(9));
    delay(1000);
  } else if (mensagem.substring(0, 6) == "SAINDO" ) {
    printLCD("ATE MAIS");
    delay(1000);
    printLCD(mensagem.substring(7));
    delay(1000);
  } else {
    printLCD("Acesso negado!");
    delay(1000);
  }
  delay(1000);
  printLCD("Passe o Cartao!");
  return;
}

//Método utilitário para print no display LCD
//Nele variamos apenas a mensagem da segunda linha
void printLCD(String mensagem) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Sitio PedeSerra");
  lcd.setCursor(0, 1);
  lcd.print(mensagem);
}

//Método de envio do id do cartão lido pra fila acesso
void sendMessage(MFRC522 mfrc522) {
  printLCD("Lendo Cartao");
  char rfidstr[15];
  char s[100];
  for (int i = 0; i < mfrc522.uid.size; i++) {

    //Conversão de byte pra Hexadecimal
    sprintf(s, "%x", mfrc522.uid.uidByte[i]);

    //Concatenando para o array de char que será enviado
    strcat( &rfidstr[i] , s);
  }
  Serial.print("Cartão ID# : ");
  Serial.print(rfidstr);

  //Publicando na fila acesso o id do cartão lido
  client.publish("acesso", rfidstr);

  Serial.println();
  printLCD("Verificando...");

  return;
}

void loop() {
  //Verificando Status do ClientMQTT
  if (!client.connected()) {
    conectMqtt();
  }
  client.loop();

  Serial.print(". ");
  //Verificando existencia do card no leitor
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    delay(1000);
    return;
  }
  Serial.print("1 ");
  //Verificando Leitura do card
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    delay(1000);
    return;
  }
  Serial.print("2 ");
  //Enviando mensagem
  Serial.print("\nEncontrado: ");
  String conteudo = "";
  byte letra;
  for (byte i = 0; i < mfrc522.uid.size; i++)
  {
    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
    conteudo.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
    conteudo.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  sendMessage(mfrc522);

  server.handleClient();
  MDNS.update();

}
String homePage() {
  return (
           "<!DOCTYPE html>"
           "<html>"
           "<meta http-equiv='refresh' content='1'/>"
           "<meta charset=\"utf-8\" />"
           "<meta name = 'viewport' content = 'width=device-width, initial-scale=1,user-scalable=0'>"
           "<title>Sitio Pe de Serra - Controle de acesso</title>"
           "<link href='https://maxcdn.bootstrapcdn.com/bootstrap/3.3.1/css/bootstrap.min.css' rel='stylesheet'></link>"
           "<p>&nbsp;</p>"
           "<div style='text-align: center;'><img src =  'http://iot-day.com/images/projects/Sitiopedeserra.png' alt='' width='290' height=63' /></div>"
           "<div class='alert alert-info' role='alert'><div style='text-align: center;'><h4>Caixa Principal</h4></div></div>"
           "<div class='col-md-6'>"
           "<meta name='viewport' content='width=device-width, initial-scale=1.0'>"
           "<span style='display: none;'>" + String(0) + "</span>"
           "<title> Colocar mensagem aqui</title>"
           "<p>&nbsp;</p>"
           "<div style='text-align: center;'>&copy 2019 - Epaminondas Lage </div>"
           "<p>&nbsp;</p>"
         );
}

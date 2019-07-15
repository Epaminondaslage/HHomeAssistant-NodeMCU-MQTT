/* ***********************************************************************
    Programa de mostrar o nível de uma caixa d'agua em uma web page
    Utiliza o processador  NodeMCU 1.0 12-E
    Necessita de uma conexão wifi
    Requer Display HD44780 com adaptador I2C, 3 boias de nível e buzzer
    Necessita disponibilizar figuras da caixa dágua com vários níveis
    em diretorio na web
    Data 30/05/2019
    Versão atualizada em  22/06/2019
    Escolher o tipo de boia a ser utilizada na variável "contato"    
 *************************************************************************/

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

// Declara SSID e Senha da rede wifi. Colocar os dados da sua rede
#ifndef STASSID
#define STASSID "pedeserra"
#define STAPSK  "planetfone"
#endif

const char* ssid = STASSID;
const char* password = STAPSK;

// Inicializa Servidor Web
ESP8266WebServer server(80);

// Ajusta o endereço do display LCD para 0x27
// Para  16 caracters e 2 linhas com interface i2c
LiquidCrystal_I2C lcd(0x27, 16, 2);



#define Pinofalante 2 // saida para buzzer
#define sensor1 14   // sensor 1
#define sensor2 12   // sensor 2
#define sensor3 13   // sensor 3
#define tempo_sirene 1
int frequencia = 0;

//Atenção ao usar boias verifique se: 
//Na posição inferior o contato é fechado, assim contato  = 0
//Na posição inferior o contato é  aberto, assim contato  = 1
boolean contato = 1;

int valor_s1 = contato, valor_s2 = contato, valor_s3 = contato;

// Simbolos para display na vertical
// Display montado na posição vertical
uint8_t letra_C[8] = {0x0, 0xE, 0x11, 0x11, 0x11, 0x11, 0x0, 0x0};
uint8_t letra_M[8] = {0x0, 0x1F, 0x2, 0x4, 0x2, 0x1F, 0x0, 0x0};
uint8_t letra_v[8] = {0x0, 0x7, 0x8, 0x10, 0x8, 0x7 , 0x0, 0x0};
uint8_t caracter_nivel[8] = {0x0, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x0};

int nivel_anterior = 0;

void handleRoot() {
  server.send(200, "html", homePage());
}

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
  server.send(404, "text/plain", message);
}

void sirene() {
  for (frequencia = 150; frequencia < 1800; frequencia += 1)
  {
    tone(Pinofalante, frequencia, tempo_sirene);
    delay(1);
  }
  for (frequencia = 1800; frequencia > 150; frequencia -= 1)
  {
    tone(Pinofalante, frequencia, tempo_sirene);
    delay(1);
  }
}

void setup() {

  // Ajusta a porta serial para 9600bps e monitorar a conexão wifi
  Serial.begin(9600);
  Serial.println(" ");
  delay(100);
  Serial.println(" ");
  delay(100);
  Serial.println("Inicializando comunicacao serial ");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  // Esperar por conexão wifi
  Serial.print ("Conectando WiFi " );
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("ok");
  //Importante : Configura NodeMCU com IP fixo. Ajustar para sua rede
  IPAddress subnet(255, 255, 255, 0);
  WiFi.config(IPAddress(10, 0, 0, 24), IPAddress(10, 0, 0, 1), subnet);
  Serial.println("IP Fixo");
 
  Serial.print("Conectado a ");
  Serial.println(ssid);
  Serial.print("Endereco IP : ");
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

  // Inicializa o display I2C
  lcd.init();

  // Imprime messagem no LCD.
  lcd.backlight();

  //Caracteres customizados
  lcd.createChar(0, letra_C);
  lcd.createChar(1, letra_M);
  lcd.createChar(2, letra_v);
  lcd.createChar(3, caracter_nivel);

  // Define os pinos dos sensores como entrada
  pinMode(sensor1, INPUT);
  pinMode(sensor2, INPUT);
  pinMode(sensor3, INPUT);

  pinMode(Pinofalante, OUTPUT);
  digitalWrite(Pinofalante, LOW);

  // Mostra a letra C no display
  lcd.setCursor(15, 0);
  lcd.write(uint8_t(0));
  // Mostra a letra M no display
  lcd.setCursor(8, 0);
  lcd.write(uint8_t(1));
  // Mostra a letra V no display
  lcd.setCursor(0, 0);
  lcd.write(uint8_t(2));

  Serial.println("Monitoracao de Caixa Dagua");
  Serial.println();
}

void loop() {

  // Controle de nível e Leitura dos sensores
  valor_s1 = digitalRead(sensor1);
  valor_s2 = digitalRead(sensor2);
  valor_s3 = digitalRead(sensor3);

  // Mostra valor dos sensores no serial monitor
  Serial.print("  Vazio S1: ");
  Serial.print(valor_s1);
  Serial.print("  Médio S2: ");
  Serial.print(valor_s2);
  Serial.print("  Cheio S3: ");
  Serial.println(valor_s3);
  delay(3000);
  // Checa o nivel e atualiza o display
  if (valor_s1 == contato && valor_s2 == contato && valor_s3 == contato)
  {
    // Atualiza o nivel no display, enviando o numero de
    // "quadrados" que devem ser mostrados para indicar o nível
    mostra_nivel(15);

  }

  int valorTotal = 0;
  if (valor_s1 == contato) {
    valorTotal = 4;
  }
  if (valor_s2 == contato && valor_s1 == contato) {
    valorTotal = 11;
  }
  if (valor_s3 == contato && valor_s2 == contato && valor_s1 == contato) {
    valorTotal = 15;
  }

  if (valorTotal != nivel_anterior) {
    mostra_nivel(valorTotal);
  }

  if (valorTotal < 11) {
    tone(Pinofalante, 1800);
  } else {
    noTone(Pinofalante);
  }

  server.handleClient();
  MDNS.update();
}

void mostra_nivel(int nivel)
{
  if (nivel != nivel_anterior)
  {
    // Apaga informacao anterior
    lcd.setCursor(0, 1);
    lcd.print("               ");
    // Atualiza o nivel no display
    int ultimo = 0;
    for (int i = 0; i <= nivel; i++)
    {
      lcd.setCursor(i, 1);
      lcd.write(uint8_t(3));
      ultimo = i;
      delay(200);
    }

    for (int u = 15; u > ultimo; u--) {
      lcd.setCursor(u, 1);
      lcd.write(' ');
      delay(200);
    }
    nivel_anterior = nivel;
  }

}

String homePage() {
  return (
           "<!DOCTYPE html>"
           "<html>"
           "<meta http-equiv='refresh' content='1'/>"
           "<meta charset=\"utf-8\" />"
           "<meta name = 'viewport' content = 'width=device-width, initial-scale=1,user-scalable=0'>"
           "<title>Sitio Pe de Serra - Caixa d'agua</title>"
           "<link href='https://maxcdn.bootstrapcdn.com/bootstrap/3.3.1/css/bootstrap.min.css' rel='stylesheet'></link>"
           "<p>&nbsp;</p>"
           "<div style='text-align: center;'><img src =  'http://iot-day.com/images/projects/Sitiopedeserra.png' alt='' width='290' height=63' /></div>"
           "<div class='alert alert-info' role='alert'><div style='text-align: center;'><h4>Caixa Principal</h4></div></div>"
           "<div class='col-md-6'>"
           "<meta name='viewport' content='width=device-width, initial-scale=1.0'>"
           "<span style='display: none;'>" + String(0) + "</span>"
           "<h1>" + ((nivel_anterior == 15) ? "<div style='text-align: center;'><img src='http://iot-day.com/images/projects/caixa_dagua_100.png' alt='' width='290' height=220' />100%</div>"
                     : (nivel_anterior == 11) ? "<div style='text-align: center;'><img src='http://iot-day.com/images/projects/caixa_dagua_maior_66.png' alt='' width='290' height=220' />70%</div>"
                     : (nivel_anterior == -1) ? "Carregando" : (nivel_anterior == 4) ? "<div style='text-align: center;'><img src='http://iot-day.com/images/projects/caixa_dagua_maior_33.png' alt='' width='290' height=220' />40%</div>"
                     : "<div style='text-align: center;'><img src='http://iot-day.com/images/projects/caixa_dagua_menor_33.png' alt='' width='290' height=220' />15%</div>") + "</h1></html>"
           "<p>&nbsp;</p>"
           "<div style='text-align: center;'>&copy 2019 - Epaminondas Lage </div>"
           "<p>&nbsp;</p>"
         );

}

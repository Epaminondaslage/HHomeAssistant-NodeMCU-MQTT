// Programa de mostrar um dado em uma página html com Node MCU 1.0 12-E
// Necessita de uma conexão wifi
// Programa: Monitoracao de Caixa d´agua usando Node MCU
// Display HD44780 com adaptador I2C
// Data 30/05/2019



#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

#ifndef STASSID
#define STASSID "123644987"
#define STAPSK  "planeta514"
#endif

const char* ssid = STASSID;
const char* password = STAPSK;

ESP8266WebServer server(80);


LiquidCrystal_I2C lcd(0x27, 16, 2); // set the LCD address to 0x27 for a 16 chars and 2 line display e interface i2c

// porque usou sensor em portas 12,13 e 15 ?

#define Pinofalante 2 // saida para buzzer
#define sensor1 14   // sensor 1
#define sensor2 12   // sensor 2
#define sensor3 13   // sensor 3
#define tempo_sirene 1
int frequencia = 0;

int valor_s1 = 1, valor_s2 = 1, valor_s3 = 1;

//Simbolos para display na vertical
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

/* void Sirene
*/

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
  // ajustar a porta serial para 9600 e monitorar
  // parte wifi

  Serial.begin(9600);
  Serial.println("Inicializando comunicacao serial ");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("  ");

  // Esperar por coneccao
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" ");
  Serial.print("Conectado a ");
  Serial.println(ssid);
  Serial.print("Endereco IP : ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);

  server.on("/inline", []() {
    server.send(200, "text/plain", "Esta funcinando bem");
  });

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server inicializado");


  // fim parte wifi

  Serial.println("Monitoracao de Caixa Dagua");

  //Inicializa o display I2C
  lcd.init();                      // initialize the lcd
  // Imprime messagem no LCD.
  lcd.backlight();
  //Caracteres customizados
  lcd.createChar(0, letra_C);
  lcd.createChar(1, letra_M);
  lcd.createChar(2, letra_v);
  lcd.createChar(3, caracter_nivel);

  //Define os pinos dos sensores como entrada
  pinMode(sensor1, INPUT);
  pinMode(sensor2, INPUT);
  pinMode(sensor3, INPUT);

  pinMode(Pinofalante, OUTPUT);
  digitalWrite(Pinofalante, LOW);

  //Mostra a letra C no display
  lcd.setCursor(15, 0);
  lcd.write(uint8_t(0));
  //Mostra a letra M no display
  lcd.setCursor(8, 0);
  lcd.write(uint8_t(1));
  //Mostra a letra V no display
  lcd.setCursor(0, 0);
  lcd.write(uint8_t(2));
  //lcd.print(ipSend);

  Serial.println("Monitoracao de Caixa Dagua");
  Serial.println();
}

void loop() {

  // controle de nível
  //Leitura dos sensores
  valor_s1 = digitalRead(sensor1);
  valor_s2 = digitalRead(sensor2);
  valor_s3 = digitalRead(sensor3);

  //Mostra valor dos sensores no serial monitor
  //Serial.print("S1: ");
  //Serial.print(valor_s1);
  //Serial.print(" S2: ");
  //Serial.print(valor_s2);
  //Serial.print(" S3: ");
  //Serial.println(valor_s3);

  //Checa o nivel e atualiza o display
  if ((valor_s1 == 0) && valor_s2 == 0 && valor_s3 == 0)
  {
    //Atualiza o nivel no display, enviando o numero de
    //"quadrados" que devem ser mostrados para indicar
    //o nivel
    mostra_nivel(15);

  }

  int valorTotal = 0;
  if (valor_s1 == 0) {
    valorTotal = 4;
  }
  if (valor_s2 == 0 && valor_s1 == 0) {
    valorTotal = 11;
  }
  if (valor_s3 == 0 && valor_s2 == 0 && valor_s1 == 0) {
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
    //Apaga informacao anterior
    lcd.setCursor(0, 1);
    lcd.print("               ");
    Serial.print(nivel);
    //Atualiza o nivel no display
    int ultimo = 0;
    for (int i = 0; i <= nivel; i++)
    {
      Serial.println(i);
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
           "<title>Sitio Pe de Serra - Caixa d'agua</title>"
           "<meta name='viewport' content='width=device-width, initial-scale=1.0'>"
           //"<h6>Sensor 1: " + String(valor_s1) + "</h6>"
           //"<h6>Sensor 2: " + String(valor_s2) + "</h6>"
           //"<h6>Sensor 3: " + String(valor_s3) + "</h6>"
           "<span style='display: none;'>" + String(0) + "</span>"
           "<style>img{max-width: 100%;}</style><div style='text-align: center;'><img src='http://caixagg.planetfone.com.br/img/sitiopedeserra.bmp'></div>"
           "<h1>" + ((nivel_anterior == 15) ? "Caixa Cheia<br><img src='http://caixagg.planetfone.com.br/img/caixa_dagua_100.png'>"
                     : (nivel_anterior == 11) ? "Caixa na Metade<br><img src='http://caixagg.planetfone.com.br/img/caixa_dagua_maior_66.png'>"
                     : (nivel_anterior == -1) ? "Carregando" : (nivel_anterior == 4) ? "Caixa está quase vazia<br><img src='http://caixagg.planetfone.com.br/img/caixa_dagua_maior_33.png'>"
                     : "Caixa Vazia<br><img src='http://caixagg.planetfone.com.br/img/caixa_dagua_menor_33.png'>") + "</h1></html>"
         );
}

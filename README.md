# Home Assistant- NodeMCU com MQTT 

## Sumário
* [Informações Gerais](#user-content-informacoes_gerais)
* [Figuras](#Figuras)
* [Drives](#Drives)
* [Setup](#setup)
* [NodeMcu e MQTT](#MQTT)
* [Status](#status)
* [Contato](#contato)

<h2 id="informacoes_gerais">Informações Gerais</h2>


Principais características da placa:

<img src="https://github.com/Epaminondaslage/NodeMCU/blob/master/Figuras/NodeMcu%20ESP-12.png" height="200" width="200">


Funções dos pinos do NodeMCU ESP12 :

Todos os pinos GPIOs, podem ser entradas ou saídas dependendo da configuração dos mesmos. Não ultrapasse a corrente de 12 mA em cada porta dessas, pois poderá danificar o chip. O recomendado é 6 mA. O nível de tensão a ser usado nessas portas não deverá ultrapassar os 3,3V. Se for necessário conectar o NodeMCU à outro dispositivo de tensão maior, como um Arduino ou módulos de 5V, use conversores bidirecionais de tensão como o do link abaixo ou use divisores resistivos de tensão.
O NodeMCU ESP12 tem duas fileiras de 15 pinos (total 30 pinos). A distância entre as duas fileiras é grande (2,30 cm), mas poderá inseri-lo em um Protoboard padrão. Os pinos RESERVED não deverão ser usados, como o nome já diz, são reservados.

    VIN – Esse é o pino de alimentação externa ( recomendo 5,0V / 1A). Pode usar até 9V, mas o regulador da placa deverá esquentar. Não use-o se estiver usando a USB.
    GND– Esse é o terra da placa. Não se esqueça de conectá-lo ao terra de outros dispositivos.
    RST – Reset do módulo ESP-12. Nível LOW(0V) dá um reboot na placa.
    EN – (Enable) ativa o módulo ESP-12 quando o nível for HIGH(3,3V).
    3.3V – saída do regulador interno 3,3V – Para alimentar outro dispositivo, não use mais do que 500 mA de corrente.
    CLK – interface SPI (clock) – pino SCLK (GPIO_6)
    SD0 – interface SPI (master in serial out) – pino MISO (GPIO_7)
    CMD – interface SPI (chip select) – pino CS (GPIO_11)
    SD1 –  interface SPI (master out serial in) – pino MOSI (GPIO_8)
    SD2 – pino GPIO_9 pode ser usado também para comunicação com SD Card (SDD2)
    SD3 – pino GIPO_10 – pode ser usado também para comunicação com SD Card (SDD3)
    RSV – reservado (não use).
    ADC0– pino de entrada do conversor analógico digital ADC de 10 bits. Tensão máxima de 1,1V (variação do valor digital – 0 a 1024).

 

    D0 – pino GIPO_16 pode ser usado para acordar (WAKE UP) o ESP8266 em modo sono profundo (Deep sleep mode).
    D1 – pino GPIO_5 – entrada ou saída.
    D2 – pino GIPO_4 – entrada ou saída.
    D3 – pino GPIO_0 é usado também para controlar o upload do programa na memória Flash. Esta conectado no botão FLASH.
    D4 – pino GPIO_2 – UART_TXD1 quando carregando o programa na memória FLASH
    D5 – pino GPIO_14  pode ser usado em SPI de alta velocidade (HSPI-SCLK)
    D6 – pino GPIO_12  pode ser usado em SPI de alta velocidade (HSPI-MISO)
    D7 – pino GPIO_13  pode ser usado em SPI de alta velocidade (HSPI-MOSI) ou UART0_CTS.
    D8 – pino GPIO_15  pode ser usado em SPI de alta velocidade (HSPI-CS) ou UART0_RTS.
    RX – pino GPIO_3 – U0RXD quando carregando o programa na memória FLASH.
    TX – pino GIPO_1 – U0TXD quando carregando o programa na memória FLASH.


## Figuras

<img src="https://github.com/Epaminondaslage/NodeMCU/blob/master/Figuras/esp8266-nodemcu-pinout.png" height="400" width="400">


## Drives

<ul>
<li>https://www.nodemcu.com/index_en.html </li>
<li>https://github.com/nodemcu/nodemcu-firmware</li>
<li>https://github.com/nodemcu/nodemcu-devkit-v1.0</li>
</ul>

## MQTT

MQTT  com NodeMCU e Home Assistant
Embora o NodeMCU possuir um chip com desempenho superior à maioria dos chips disponíveis no mercado para a mesma finalidade, ele ainda é um processador bastante limitado e precisa trabalhar com protocolos leves, como o MQTT, para fornecer um desempenho satisfatório.
A configuração do broker para a arquitetura publish-subscribe e o uso do MQTT requerem apenas a instalação de um servidor MQTT. Com um broker devidamente instalado, os tópicos aos quais os sensores e atuadores publicam e se inscrevem serão criados e apagados dinamicamente, de acordo com o seu uso e configuração. Atualmente, há vários servidores MQTT de código aberto no mercado, o Mosquitto, da Eclipse Foundation será utilizado no Home Assistant.
Para gerenciar os objetos conectados ao broker e as conexões, pacotes de controle são transmitidos pela rede. Através desses pacotes, é garantido que todas as mensagens enviadas cheguem ao broker e torna-se possível estabelecer níveis de QoS (Quality of Service). Os principais pacotes trafegados entre os objetos são:
<ul>
<li></li>CONNECT: Enviado pelo cliente para se conectar ao broker.
<li>CONNACK: Reconhecimento da solicitação de conexão.
<li>PUBLISH: Enviado pelo cliente para o broker, publicando uma mensagem.
<li>PUBACK: Reconhecimento da solicitação de publicação.
<li>PUBREC: Publicação recebida. QoS 0: a mensagem não é armazenada. A mensagem é entregue no máximo uma vez ou não é entregue. Pode ser perdida se o cliente for desconectado ou se o servidor falhar.
<li>PUBREL: Publicação publicada. QoS 1: modo de transferência padrão. A mensagem é sempre entregue pelo menos uma vez. Se o emissor não receber uma confirmação, a mensagem será enviada novamente.
<li>PUBCOMP: Publicação completada. QoS 2: a mensagem é sempre entregue exatamente uma vez e é armazenada localmente no emissor e no receptor até ser processada.
<li>SUBSCRIBE: Enviado para se inscrever em determinado tópico.
<li>SUBACK: Reconhecimento da inscrição.
<li>UNSUBSCRIBE: Enviado para cancelar a inscrição em determinado tópico.
<li>UNSUBACK: Reconhecimento do cancelamento de inscrição.
<li>DISCONNECT: Enviado para se desconectar do broker.

Para facilitar a utilização do protocolo MQTT com o NodeMCU, existe a biblioteca de código aberto "PubSubClient". Com essa biblioteca, é possível trocar mensagens MQTT com um broker de forma simplificada. Além do MQTT, existem outras bibliotecas de código aberto que ajudam a tratar e implementar os protocolos necessários para conectar e aumentar a precisão do sensoriamento do NodeMCU. Algumas das bibliotecas amplamente utilizadas e em constante produção incluem: ESP8266WiFi, WiFiManager, ArduinoOTA e Bounce2




## Setup
Para Instalar o NodeMCU no Windows seguir o tutorial:</P>
http://blog.eletrogate.com/nodemcu-esp12-usando-arduino-ide-2/


## Status
Este repositório é para alunos do Curso de Eletrotécnica do CEFET-MG 

## Contato
Criado/adaptado por Epaminondas de Souza  Lage - epaminondaslage@gmail.com ou epaminondaslage@cefetmg.br


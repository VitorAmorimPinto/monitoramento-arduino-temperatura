#include <Wire.h>
#include <SPI.h>
#include <Ethernet.h>

// Endereço I2C do sensor DS1621
#define DS1621_ADDRESS 0x48

//define o ip e porta do servidor
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192, 168, 25, 16);
EthernetServer server(8080);

void setup() {
    Serial.begin(9600);
    Wire.begin();
    Ethernet.begin(mac, ip);
    server.begin();
    initializeDS1621();
    enviarAlerta();
}

void loop() {
  float temperature = lerTemperatura();

  String req_str;
  EthernetClient client = server.available();
  if (client) {
    req_str = "";
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        req_str += c;
        if (c == '\n' && currentLineIsBlank) {

          unsigned int posicao_inicial = req_str.indexOf("GET") + 4;
          unsigned int posicao_final = req_str.indexOf("HTTP/") - 1;
          String parametro = req_str.substring(posicao_inicial, posicao_final);

          if (parametro != "/favicon.ico") {
          }

          if (parametro.indexOf("temperatura") > -1) {
            // Construir a resposta JSON
            float temperature = lerTemperatura();
            
            String temperatureString = String(temperature);

            String jsonResponse = "{\"temperatura\": "+temperatureString+"}";

            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: application/json");
            client.println("Connection: close");
            client.println();
            client.println(jsonResponse);
          }
          break;
        }
        if (c == '\n') {
          currentLineIsBlank = true;
        } else if (c != '\r') {
          currentLineIsBlank = false;
        }
      }
    }
    delay(1);
    client.stop();
  }
}
void initializeDS1621() {
  Wire.beginTransmission(DS1621_ADDRESS);
  Wire.write(0xAC);  // Código de comando para ativar o sensor
  Wire.write(0x0C);  // Configuração para modo de 1 leitura por segundo
  Wire.endTransmission();
}
void enviarAlerta() {
  Serial.println("Alerta Enviado");
  EthernetClient client;
  if (client.connect("127.0.0.1", 8686)) {
    client.print("GET /alertar HTTP/1.1\r\n");
    client.print("Host: 127.0.0.1\r\n");
    client.print("Connection: close\r\n");
    client.print("\r\n");
    client.stop();
  }
}
float lerTemperatura() {
  Wire.beginTransmission(DS1621_ADDRESS);
  Wire.write(0xAA);  // Código de comando para ler a temperatura
  Wire.endTransmission();
  
  delay(10);  // Aguarda a leitura ser concluída

  Wire.requestFrom(DS1621_ADDRESS, 2);
  byte msb = Wire.read();
  byte lsb = Wire.read();
  
  // Calcula a temperatura em graus Celsius
  float temperature = ((msb << 8) | lsb) / 256.0;

  return temperature;
}
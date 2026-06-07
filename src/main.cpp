#include <Arduino.h>
#include <WiFi.h>

const char* SSID = "seu wifi";
const char* senha = "sua senha do wifi";

WiFiServer  servidorTCP(8080);

void TaskConexao(void *pv){
  WiFi.begin(SSID,senha);
  Serial.println("[WIFI]  Conectando");

  while (1)
  {
    if(WiFi.status() != WL_CONNECTED){
      Serial.print(".");
      vTaskDelay(pdMS_TO_TICKS(500));
    }
    else{
      Serial.println("\n[WIFI] Conectado com Sucesso!");
      Serial.print("[WIFI] O IP do esp32 é: ");
      Serial.println(WiFi.localIP());
      servidorTCP.begin();
      vTaskDelay(pdMS_TO_TICKS(10000));
    }
  }  
}

void TaskEscutaTCP(void *pv){
  while (1)
  {
    WiFiClient cliente = servidorTCP.available();

    if(cliente){
      Serial.println("[TCP] Novo cliente conectado!");

      cliente.println("Bem-Vindo so Servidor do ESP32");
      cliente.println("Digite uma mensgem e aperte ENTER:");

      while (cliente.connected())
      {
        /* code */
        if(cliente.available()){
          String mensagemRecebida = cliente.readStringUntil('\n');

          Serial.print("[CLIENTE]:");
          Serial.println(mensagemRecebida);

          cliente.print("ESP32 recebeu essa mensagem:");
          cliente.println(mensagemRecebida);
        }
        vTaskDelay(pdMS_TO_TICKS(10));
      }
      cliente.stop();
      Serial.println("[TCP] Cliente desconectou");
    }
    vTaskDelay(pdMS_TO_TICKS(100));
  }  
}

void setup() { 
  Serial.begin(115200);
  vTaskDelay(pdMS_TO_TICKS(5000));

  xTaskCreate(TaskConexao,"conexao", 4096,NULL,1,NULL);
  xTaskCreate(TaskEscutaTCP,"tcp", 4096,NULL,2,NULL);
}

void loop() {
  // put your main code here, to run repeatedly:
}
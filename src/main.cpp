#include <Arduino.h>
#include <Wire.h>

#define pin_SDA 23
#define pin_SCL 21

//controlador do semaforo (chave do banheiro)
SemaphoreHandle_t xMutexI2C = NULL;

void Taski2c(void *pv){
  while (1)
  {
    /* code */
    Serial.println("[SCANNER] Tentando obter chave do semáforo...");

    if(xSemaphoreTake(xMutexI2C, pdMS_TO_TICKS(1000))==pdTRUE){
      Serial.println("[SCANNER] Chave Obtida. Iniciando Varredura ...");

      byte  erro,endereco;
      int TotDispositivos = 0;

      for(endereco = 1; endereco <127; endereco++){
        Wire.beginTransmission(endereco);
        erro = Wire.endTransmission();

        if(erro == 0){
          Serial.printf("[I2C] -> Dispositovo no endereço: 0x%02X\n", endereco);
          TotDispositivos++;
        }
      }
      if(TotDispositivos ==0){
        Serial.println("[I2C] Nenhum dispositivo respondendo nos pinos.");
      }

      Serial.println("[SCANNER] Varredura concluída. Devolvendo chave...");
      xSemaphoreGive(xMutexI2C);
    }
    else{
      Serial.println("Não consegui pegar a chave do semaforo");
    }
    vTaskDelay(pdMS_TO_TICKS(10000));
  }  
}

void setup() { 
  Serial.begin(115200);

  vTaskDelay(pdMS_TO_TICKS(2000));
  Serial.println("--- Inicializando Barramento I2C com Protecao FreeRTOS ---");
  Wire.begin(pin_SDA,pin_SCL);

  xMutexI2C = xSemaphoreCreateMutex();
  if(xMutexI2C != NULL){
    xTaskCreate(Taski2c, "i2c", 3072,NULL,1,NULL);
  }
  else{
    Serial.println("Não consegui criar mutex");
  }
}

void loop() {
  // put your main code here, to run repeatedly:
}
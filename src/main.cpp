#include <Arduino.h>
#include <Wire.h>

//pinos do I2C
#define pin_SDA 23
#define pin_SCL 21
#define mpu_addr  0x68 // endereço I2C do MPU6050
#define reg_pwr_mgmt_1 0x6B //registrador de energia do 6050
#define reg_accel_xout_h  0x3B //registrador que começa os dados do 6050

//controlador do semaforo (chave do banheiro)
SemaphoreHandle_t xMutexI2C = NULL;

void Acordar6050(){
  Wire.beginTransmission(mpu_addr);
  Wire.write(reg_pwr_mgmt_1);
  Wire.write(0x00);
  Wire.endTransmission();
  Serial.println("[6050] SENSOR ACORDADO!");
}

void TaskLeituraAcelerometro(void* pv){
  while (1)
  {
    /* code */
    if(xSemaphoreTake(xMutexI2C,pdMS_TO_TICKS(100))==pdTRUE){
      Wire.beginTransmission(mpu_addr); //abre conexão com o 6050
      Wire.write(reg_accel_xout_h);
      Wire.endTransmission(false);// O 'false' envia um RESTART no barramento, mantendo a linha presa

      Wire.requestFrom(mpu_addr,6);

      if(Wire.available() == 6){
        int16_t rawX = (Wire.read()<<8 | Wire.read()); //a informação tem 16bit e o registrador le de 8, então se le 8, desloca pra esquerda e le os proximos 8 bits
        int16_t rawY = (Wire.read()<<8 | Wire.read());
        int16_t rawZ = (Wire.read()<<8 | Wire.read());

        Serial.printf("[6050] X: %6d | Y: %6d | Z: %6d\n", rawX,rawY,rawZ);
      }

      xSemaphoreGive(xMutexI2C);
    }
    else{
      Serial.println("[AVISO] task não consegui acesso a chave do banheiro");
    }
    vTaskDelay(pdMS_TO_TICKS(500));
  }  
}


void setup() { 
  Serial.begin(115200);
  vTaskDelay(pdMS_TO_TICKS(5000));

  Serial.println("--- Inicializando Leitura MPU6050 via FreeRTOS ---");

  Wire.begin(pin_SDA,pin_SCL);

  xMutexI2C = xSemaphoreCreateMutex();
  if(xMutexI2C != NULL){
    Acordar6050();
    xTaskCreate(TaskLeituraAcelerometro,"6050", 3072, NULL, 2, NULL);
  }else {
    Serial.println("Erro ao criar o Mutex!");
  }
}

void loop() {
  // put your main code here, to run repeatedly:
}
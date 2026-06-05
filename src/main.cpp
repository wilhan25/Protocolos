#include <Arduino.h>

#define pin_tx  21
#define pin_rx  18

void Task_TX(void *pv){
  int contador = 0;
  char mensagem[32];

  while (1)
  {
    /* code */
    sprintf(mensagem, "pacote IoT numero: %d", contador++);
    Serial.print("[TX] Enviando pela UART1: ");
    Serial.println(mensagem);

    Serial1.println(mensagem);
    vTaskDelay(3000/portTICK_PERIOD_MS);
  }  
}

void Task_RX(void *pv){
  while (1)
  {
    if (Serial1.available() > 0)
    {
      Serial.print("[RX] -> Opa, chegou alguma coisa aqui ! Mensagem: ");

      String msgRecebida = Serial1.readStringUntil('\n');
      Serial.println(msgRecebida);
    }
    vTaskDelay(50/portTICK_PERIOD_MS);    
  }  
}

void setup() { 
  Serial.begin(115200);
  vTaskDelay(5000/portTICK_PERIOD_MS);

  Serial.println("--- Inicializando Teste de Loopback UART com FreeRTOS ---");

  Serial1.begin(115200, SERIAL_8N1, pin_rx, pin_tx);

  xTaskCreate(Task_TX,"tx",2048,NULL,1,NULL);
  xTaskCreate(Task_RX,"rx",2048, NULL,1,NULL);
}

void loop() {
  // put your main code here, to run repeatedly:
}
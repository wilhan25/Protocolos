#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h> // Biblioteca nativa do ESP32 para Web

// Configurações do seu Wi-Fi
const char* SSID = "NOME_DA_SUA_REDE";
const char* PASSWORD = "SENHA_DA_SUA_REDE";

// URL de teste (HTTP puro, sem SSL para facilitar nossa base)
const char* URL_SERVIDOR = "http://httpbin.org/get";

// Protótipos das Tasks
void TaskWiFi(void *pvParameters);
void TaskRequisicaoHTTP(void *pvParameters);

void setup() {
    Serial.begin(115200);
    vTaskDelay(pdMS_TO_TICKS(1000));
    Serial.println("\n--- Iniciando Cliente HTTP ---");

    // Criação das Tasks no FreeRTOS
    xTaskCreate(TaskWiFi,           "WiFi_Task", 4096, NULL, 1, NULL);
    xTaskCreate(TaskRequisicaoHTTP, "HTTP_Task", 8192, NULL, 2, NULL); // HTTP exige mais memória RAM (Stack)
}

void loop() {
    // FreeRTOS no comando
}

// TASK 1: Garante que o Wi-Fi esteja sempre conectado
void TaskWiFi(void *pvParameters) {
    WiFi.begin(SSID, PASSWORD);
    Serial.print("[Wi-Fi] Conectando");

    for(;;) {
        if (WiFi.status() != WL_CONNECTED) {
            Serial.print(".");
            vTaskDelay(pdMS_TO_TICKS(500));
        } else {
            vTaskDelay(pdMS_TO_TICKS(5000)); // Checa a conexão a cada 5 segundos
        }
    }
}

// TASK 2: Faz o "Bate e Volta" com o servidor web
void TaskRequisicaoHTTP(void *pvParameters) {
    for(;;) {
        // Só tenta fazer a requisição se o Wi-Fi estiver conectado
        if (WiFi.status() == WL_CONNECTED) {
            Serial.println("\n[HTTP] Preparando requisição GET...");

            HTTPClient http;
            WiFiClient clienteTCP; // O Socket TCP que aprendemos na primeira aula!

            // 1. Abre a conexão TCP na porta 80 (padrão HTTP) com o servidor
            http.begin(clienteTCP, URL_SERVIDOR);

            // 2. Dispara o gatilho do pedido (Requisição GET)
            int codigoStatus = http.GET();

            // 3. Verifica o que o servidor respondeu
            if (codigoStatus > 0) {
                Serial.printf("[HTTP] Código de Status do Servidor: %d\n", codigoStatus);

                // Se o código for 200 (OK), nós lemos o conteúdo que o servidor mandou
                if (codigoStatus == HTTP_CODE_OK) {
                    String respostaServidor = http.getString();
                    Serial.println("[HTTP] Resposta recebida do servidor:");
                    Serial.println(respostaServidor); // Vai printar um texto formatado em JSON
                }
            } else {
                Serial.printf("[HTTP] Erro ao enviar requisição: %s\n", http.errorToString(codigoStatus).c_str());
            }

            // 4. Fecha a conexão e libera a memória RAM do ESP32
            http.end();
        } else {
            Serial.println("[HTTP] Aguardando conexão Wi-Fi...");
        }

        // Aguarda 10 segundos antes de fazer a próxima requisição
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}
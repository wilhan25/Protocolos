#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h> // A biblioteca mágica do MQTT

// Configurações de Rede e MQTT
const char* SSID = "NOME_DA_SUA_REDE";
const char* PASSWORD = "SENHA_DA_SUA_REDE";
const char* BROKER_MQTT = "broker.hivemq.com"; // Broker público e gratuito na nuvem
const int BROKER_PORT = 1883; // Porta padrão mundial do MQTT (sem criptografia)

// Objetos de Rede
WiFiClient espClient;           // O Socket TCP base (que aprendemos agora pouco!)
PubSubClient clienteMQTT(espClient); // O motor MQTT rodando em cima do TCP

// Protótipos das Tasks e Funções
void TaskManterConexoes(void *pvParameters);
void TaskPublicarSensor(void *pvParameters);
void callbackRecebimentoMQTT(char* topico, byte* payload, unsigned int tamanho);

void setup() {
    Serial.begin(115200);
    vTaskDelay(pdMS_TO_TICKS(1000));

    // Configura o motor MQTT apontando para o servidor na nuvem
    clienteMQTT.setServer(BROKER_MQTT, BROKER_PORT);
    // Diz qual função deve ser chamada quando chegar uma mensagem para o ESP32
    clienteMQTT.setCallback(callbackRecebimentoMQTT);

    // Cria as Tasks no FreeRTOS
    xTaskCreate(TaskManterConexoes, "WiFi_MQTT", 4096, NULL, 2, NULL);
    xTaskCreate(TaskPublicarSensor, "Publicador", 3072, NULL, 1, NULL);
}

void loop() {
    // Vazio.
}

// TASK 1: O Coração - Mantém o Wi-Fi e o Broker conectados
void TaskManterConexoes(void *pvParameters) {
    for(;;) {
        // 1. Checa Wi-Fi
        if (WiFi.status() != WL_CONNECTED) {
            Serial.println("[Rede] Conectando ao Wi-Fi...");
            WiFi.begin(SSID, PASSWORD);
            while (WiFi.status() != WL_CONNECTED) { vTaskDelay(pdMS_TO_TICKS(500)); }
            Serial.println("[Rede] Wi-Fi Conectado!");
        }

        // 2. Checa MQTT
        if (WiFi.status() == WL_CONNECTED && !clienteMQTT.connected()) {
            Serial.println("[MQTT] Conectando ao Broker HiveMQ...");
            // Cria um ID único para esse ESP32 não dar conflito na nuvem
            String idCliente = "ESP32_Wilhan_" + String(random(0xffff), HEX);
            
            if (clienteMQTT.connect(idCliente.c_str())) {
                Serial.println("[MQTT] Conectado ao Broker!");
                // No instante que conecta, se INSCREVE no tópico de controle do LED
                clienteMQTT.subscribe("wilhan/embarcados/led");
                Serial.println("[MQTT] Inscrito no tópico: wilhan/embarcados/led");
            } else {
                Serial.printf("[MQTT] Falha ao conectar. Status: %d\n", clienteMQTT.state());
            }
        }

        // 3. Função vital: Mantém os pings de conexão em background e escuta mensagens novas
        if (clienteMQTT.connected()) {
            clienteMQTT.loop();
        }

        vTaskDelay(pdMS_TO_TICKS(100)); // Roda a checagem 10 vezes por segundo
    }
}

// TASK 2: Simula a leitura de um sensor de temperatura industrial e publica
void TaskPublicarSensor(void *pvParameters) {
    int contadorTemperatura = 25;

    for(;;) {
        if (clienteMQTT.connected()) {
            // Prepara a mensagem como texto
            char mensagem[10];
            sprintf(mensagem, "%d", contadorTemperatura);

            // Publica o dado no tópico para o mundo (nuvem)
            clienteMQTT.publish("wilhan/embarcados/sensor", mensagem);
            Serial.printf("[TX] Publiquei temperatura: %d no tópico wilhan/embarcados/sensor\n", contadorTemperatura);

            contadorTemperatura++;
            if(contadorTemperatura > 40) contadorTemperatura = 25;
        }
        
        // Espera 5 segundos para publicar novamente (economiza banda da IoT)
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

// FUNÇÃO DE INTERRUPÇÃO (CALLBACK): Acordada quando o Broker envia algo para nós
void callbackRecebimentoMQTT(char* topico, byte* payload, unsigned int tamanho) {
    Serial.printf("\n[RX] Mensagem recebida do Broker no Tópico: %s\n", topico);
    Serial.print("[RX] Comando: ");
    
    // Converte os bytes puros em um texto legível
    String comando = "";
    for (int i = 0; i < tamanho; i++) {
        comando += (char)payload[i];
    }
    Serial.println(comando);

    // Lógica para ligar um atuador
    if (comando == "LIGAR") {
        Serial.println(">> Ação: Ligando o Relé/LED de hardware!\n");
    } else if (comando == "DESLIGAR") {
        Serial.println(">> Ação: Desligando o Relé/LED de hardware!\n");
    }
}
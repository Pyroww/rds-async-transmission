#include <Wire.h>
#include <RDA5807.h>

RDA5807 rx;

char ultimaMensagem[65] = ""; 
unsigned long tempoUltimaChegada = 0; 

void limparTexto(char* texto) {
  if (texto == NULL) return;
  for (int i = 0; i < strlen(texto); i++) {
    if (!isprint(texto[i])) texto[i] = ' '; 
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("SYS_ID:RECEPTOR");
  Serial.println("\n--- RECEPTOR RDA5807M (PULL-UP D4/D3) COM TIMER ---");

  // Força o barramento I2C a iniciar nos pinos com resistores embutidos
  Wire.begin(D4, D3); 
  
  // Inicialização eletrônica
  rx.setup();
  
  // Sintonia e ganho
  rx.setFrequency(10610); // 106.1 MHz
  rx.setVolume(15);
  rx.setRDS(true);

  Serial.println("✅ Módulo sintonizado e operando em 106.1 MHz.");
  Serial.println("---------------------------------------------------------");
}

void loop() {
  if (Serial.available() > 0) {
    String comando = Serial.readStringUntil('\n');
    comando.trim();
    if (comando == "PING_ID") Serial.println("SYS_ID:RECEPTOR");
  }
  
  if (rx.getRdsReady()) { 
    // CORREÇÃO: Lendo o Grupo 2A (Radio Text - 64 caracteres) em vez do Grupo 0A (Nome da Estação)
    // Caso a biblioteca PU2CLR RDA5807 dê aviso de método não encontrado, use: rx.getRdsText();
    char* radioText = rx.getRdsText2A(); 

    if (radioText != NULL) {
      limparTexto(radioText);
      
      // Filtro para não printar lixo vazio e não repetir a mesma mensagem freneticamente
      if (strlen(radioText) > 0 && strcmp(radioText, ultimaMensagem) != 0) {
        strcpy(ultimaMensagem, radioText);
        
        unsigned long tempoAtual = millis();
        unsigned long deltaChegada = tempoAtual - tempoUltimaChegada;
        tempoUltimaChegada = tempoAtual;

        Serial.print("⏱️ [");
        Serial.print(tempoAtual);
        Serial.print(" ms] (Delta: ");
        Serial.print(deltaChegada);
        Serial.print(" ms) | 📝 Mensagem Decodificada: [");
        Serial.print(ultimaMensagem);
        Serial.println("]");
      }
    }
  }
  delay(10);
}
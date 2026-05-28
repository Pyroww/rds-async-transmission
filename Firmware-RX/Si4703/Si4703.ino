#include <Wire.h>
#include <SI470X.h>

#define PINO_RESET D5 

SI470X rx;

char ultimaMensagem[65] = ""; 
char ultimaEstacao[10] = ""; 
unsigned long tempoUltimaChegada = 0; // Armazena o momento do último pacote

void limparTexto(char* texto) {
  if (texto == NULL) return;
  for (int i = 0; i < strlen(texto); i++) {
    if (!isprint(texto[i])) texto[i] = ' '; 
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("SYS_ID:RECEPTOR");
  Serial.println("\n--- RECEPTOR SI4703 (STRING BÁSICA) COM TIMER ---");

  pinMode(PINO_RESET, OUTPUT);
  digitalWrite(PINO_RESET, LOW); delay(100); 
  digitalWrite(PINO_RESET, HIGH); delay(100); 

  Wire.begin(D2, D1); 
  rx.setup(PINO_RESET, D2);
  
  rx.setFrequency(10610);
  rx.setVolume(15);
  rx.setRDS(true);

  Serial.println("✅ Rádio sintonizado em 106.1 MHz.");
  Serial.println("---------------------------------------------------------");
}

void loop() {
  if (Serial.available() > 0) {
    String comando = Serial.readStringUntil('\n');
    comando.trim();
    if (comando == "PING_ID") Serial.println("SYS_ID:RECEPTOR");
  }
  
  if (rx.getRdsReady()) { 
    char* radioText = rx.getRdsText2A();

    if (radioText != NULL) {
      limparTexto(radioText);
      
      // Se a mensagem for diferente da anterior, significa que um comando novo chegou!
      if (strcmp(radioText, ultimaMensagem) != 0) {
        strcpy(ultimaMensagem, radioText);
        
        // --- ⏱️ REGISTRO DE TEMPO DE CHEGADA ---
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
}

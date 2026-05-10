#include <Wire.h>
#include <SI470X.h>

#define PINO_RESET D5 

SI470X rx;

// Memórias separadas: uma para a Mensagem, outra para a Estação
char ultimaMensagem[65] = ""; 
char ultimaEstacao[10] = ""; 


// "VASSOURA DIGITAL"
void limparTexto(char* texto) {
  if (texto == NULL) return;
  for (int i = 0; i < strlen(texto); i++) {
    if (!isprint(texto[i])) { 
      texto[i] = ' '; 
    }
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("SYS_ID:RECEPTOR");
  Serial.println("\n--- RECEPTOR SI4703: ESCUTA DUPLA (ESTAÇÃO + MENSAGEM) ---");

  pinMode(PINO_RESET, OUTPUT);
  digitalWrite(PINO_RESET, LOW);
  delay(100); 
  digitalWrite(PINO_RESET, HIGH); 
  delay(100); 

  Wire.begin(D2, D1); 
  rx.setup(PINO_RESET, D2);
  
  rx.setFrequency(10610);
  rx.setVolume(15);
  rx.setRDS(true);

  Serial.println("✅ Rádio sintonizado em 106.1 MHz.");
  Serial.println("📡 Aguardando pacotes de Estação e Mensagem...");
  Serial.println("---------------------------------------------------------");
}

void loop() {
  // Fica escutando se o Java pergunta quem ele é
  if (Serial.available() > 0) {
    String comando = Serial.readStringUntil('\n');
    comando.trim();
    if (comando == "PING_ID") {
      Serial.println("SYS_ID:RECEPTOR");
    }
  }
  
  if (rx.getRdsReady()) { 
    
    char* stationName = rx.getRdsText0A();
    char* radioText = rx.getRdsText2A();

    bool houveAtualizacao = false;

    // 1. Verifica se a Estação chegou/mudou
    if (stationName != NULL) {
      limparTexto(stationName);
      if (strcmp(stationName, ultimaEstacao) != 0) {
        strcpy(ultimaEstacao, stationName);
        houveAtualizacao = true;
      }
    }

    // 2. Verifica se a Mensagem chegou/mudou
    if (radioText != NULL) {
      limparTexto(radioText);
      if (strcmp(radioText, ultimaMensagem) != 0) {
        strcpy(ultimaMensagem, radioText);
        houveAtualizacao = true;
      }
    }

    // Se qualquer um dos dois teve atualização, imprime o painel completo
    if (houveAtualizacao) {
      Serial.print("📻 Estação: [");
      Serial.print(strlen(ultimaEstacao) > 0 ? ultimaEstacao : "Aguardando...");
      Serial.print("] | 📝 Mensagem: [");
      Serial.print(strlen(ultimaMensagem) > 0 ? ultimaMensagem : "Aguardando...");
      Serial.println("]");
    }
  }
}
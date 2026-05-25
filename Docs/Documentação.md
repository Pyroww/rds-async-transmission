# 📚 Documentação Técnica do uso de Infraestrutura FM como Canal de Controle por RDS

Esta documentação técnica descreve a arquitetura de baixo nível, os protocolos de sinalização e os algoritmos lógicos implementados no ecossistema de comunicação *Out-of-Band* (OOB). O sistema viabiliza o transporte unidirecional assíncrono de cadeias de caracteres (*strings* de controle) utilizando a subportadora de 57 kHz de emissoras de rádio FM convencional por meio do protocolo RDS (Radio Data System).

---

## 1. Arquitetura Geral do Sistema e Camadas

O ecossistema é projetado sob um modelo de três camadas físicas e lógicas distintas:
1. **Camada de Aplicação e Orquestração (Java/Host):** Gerencia a injeção central de payloads e monitora os terminais de forma assíncrona a 115.200 bps.
2. **Camada de Enlace e Processamento Local (Microcontroladores):** Gateways baseados em ESP32 (Transmissor) e ESP8266 (Receptores) que executam rotinas lógicas de inicialização, barramento e sanitização de memória.
3. **Camada Física de Radiofrequência (RF):** Composta pelos transmissores e receptores integrados de rádio (Si4713 para modulação e Si4703 ou RDA5807M para demodulação) operando em onda portadora central de 106.1 MHz.

## 2. Protocolo de Sincronismo e Handshake Ativo (Serial-USB)

O canal de comunicação estabelecido entre o computador host (Orquestrador Java) e os microcontroladores utiliza comunicação serial assíncrona assinalada a 115.200 baud.

### 2.1. Fluxo de Inicialização e Identificação de Topologia

Durante o processo de inicialização do barramento serial, os microcontroladores informam ativamente o seu perfil de hardware por meio da emissão de strings identificadoras exclusivas `(SYS_ID)` no buffer de saída:

* Nó Transmissor (ESP32): Emite SYS_ID:TRANSMISSOR imediatamente após a execução do bloco de inicialização.

* Nós Receptores (ESP8266): Emitem SYS_ID:RECEPTOR ao inicializar a rotina de boot.

### 2.2. Varredura e Polling Assíncrono (PING_ID)
Para evitar conflitos operacionais, os nós receptores incorporam uma escuta serial ativa no laço principal (void loop()). Ao interceptarem a instrução de checagem PING_ID\n enviada pelo host, o firmware responde imediatamente com a string de controle de estado SYS_ID:RECEPTOR. Essa lógica permite ao Orquestrador Java diferenciar os nós mesmo em conexões alternadas na mesma porta física COM.

## 3. Especificação do Nó transmissor (ESP32 + Si4713)
O nó transmissor atua como um gateway tradutor entre comandos estruturados via USB e pacotes de modulação em quadratura injetados na subportadora FM.

### 3.1. Pinagem e Conexões Físicas
| ESP32 Pin | Si4713 Pin | Função |
| :--- | :--- | :--- |
| 3V3 (GPIO 3V3) | VIN | Alimentação Elétrica (3.3V) |
| GND (GPIO GND) | GND | Referência de Aterramento |
| D21 (GPIO 21) | SDA | Linha de Dados do Barramento I2C |
| D22 (GPIO 22) | SCL | Linha de Clock do Barramento I2C |
| D27 (GPIO 27) | RST / RESET | Pino de Controle de Reset de Hardware |

### 3.2. Ciclo de Reset de Hardware Forçado (Power Cycle)

Para mitigar travamentos de registradores internos do chip Si4713 decorrentes de flutuações elétricas no barramento I2C, o firmware adota um procedimento de reinicialização galvânica controlada no `void setup()` antes de instanciar a biblioteca controladora:

1. Configura o RESET_PIN (GPIO 27) como saída digital.

2. Força a linha de reset a nível lógico baixo (LOW), drenando o pino para o terra por 100 ms.

3. Eleva a linha de reset para nível lógico alto (HIGH), energizando a lógica interna do chip e aguarda um tempo de estabilização de 50 ms para a conclusão do boot do periférico

### 3.3. Configuração de RF e Inicialização do RDS
Após o reset, o barramento I2C é inicializado explicitamente nos pinos nativos do ESP32 (Wire.begin(21, 22)). O chip é parametrizado com as seguintes constantes:

* Potência de Transmissão (TX Power): Configurada no limite estável de 115 dBµV.

* Frequência Central: Sintonizada em 106.1 MHz (10610).

* Estação de Serviço (PS): Definida estaticamente como "RADIO   ".

* Buffer Inicial: Instanciado com a mensagem padrão de espera de 64 caracteres preenchida com espaços em branco.

## 4. Especificações dos Nós receptores (ESP8266)

O sistema valida a resiliência do protocolo RDS por meio de dois firmwares intercambiáveis baseados no microcontrolador ESP8266, otimizados para operar com diferentes chips comerciais de recepção. Ambos os nós utilizam a leitura do Grupo 2A (Radio Text), garantindo a captura estável de blocos de mensagens longas de até 64 caracteres.

### 4.1. Variante A (principal): Receptor Si4703
Esta variante implementa um barramento I2C com controle síncrono de inicialização elétrica através de um pino dedicado de reset controlado.

### 4.1.1. Tabela de Pinagem (Si4703)
| ESP8266 Pin | Si4703 Pin | Função |
| :--- | :--- | :--- |
| 3V3 | 3.3V | Alimentação Lógica (3.3V) |
| GND | GND | Referência de Terra |
| D2 (GPIO 4) | SDIO / SDA | Dados do Barramento I2C |
| D1 (GPIO 5) | SCLK / SCL | Clock do Barramento I2C |
| D5 (GPIO 14) | RST | Linha de Reset de Hardware dedicada |

### 4.1.2. Rotina de Inicialização e Sequenciamento de Boot (Si4703)

Diferente do RDA5807M, o Si4703 requer um chaveamento elétrico rígido para selecionar o modo de operação I2C (2-wire) durante o boot. O firmware realiza a seguinte sequência lógica:

1. Define o pino D5 como saída digital.

2. Derruba o pino para LOW por 100 ms (Mantém o transceptor em estado de suspensão).

3. Eleva o pino para HIGH por 100 ms (Desperta o dispositivo).

4. Inicializa o barramento nos pinos lógicos definidos via software: Wire.begin(D2, D1).

5. Aciona o método rx.setup(PINO_RESET, D2), consolidando a amarração dos pinos logados e sintonizando a frequência alvo em 106.1 MHz (10610) com volume máximo (15).

### 4.2. Variante B (secundário): receptor RDA5807M
Esta variante dispensa pinos dedicados de reset de hardware e faz uso dos resistores de pull-up internos nativos da placa de desenvolvimento (ESP8266).

### 4.2.1. Tabela de Pinagem (RDA5807M)
| ESP8266 Pin | RDA 5807M Pin | Função |
| :--- | :--- | :--- |
| 3V3 | VIN | Alimentação Lógica (3.3V) |
| GND | GND | Referência de Terra |
| D3 (GPIO 0) | SCL | Clock I2C (Possui Resistor Pull-up integrado) |
| D4 (GPIO 2) | SDA | Dados I2C (Possui Resistor Pull-up integrado) |

## 5. Engenharia de Algoritmos Críticos (Firmware)
A resiliência da comunicação e a integridade das strings de controle transmitidas dependem de três algoritmos implementados a nível de firmware nos microcontroladores:

### 5.1. Algoritmo de Padding Dinâmico (Nó Transmissor)
O registrador de Radio Text (2A) do protocolo RDS opera com pacotes cíclicos de tamanho fixo. O envio de strings de comprimentos variáveis causa corrupção de memória e sobreposição de caracteres antigos no visor do receptor. Para anular este efeito, o ESP32 gerencia o tamanho do payload injetado através de processamento de string em tempo real:

```
// Garante o preenchimento exato de 64 bytes com espaços vazios
while (payloadJava.length() < 64) {
    payloadJava += " ";
}

// Trunca o payload se houver estouro do limite físico de memória
if (payloadJava.length() > 64) {
    payloadJava = payloadJava.substring(0, 64);
}

radio.setRDSbuffer(payloadJava.c_str());
```

### 5.2. Filtro digital (limparTexto)
A subportadora de 57 kHz modulada em FM é altamente vulnerável a ruídos impulsivos e interferências eletromagnéticas (EMI). Esses fenômenos distorcem os bits demodulados, inserindo caracteres de controle não-imprimíveis ou lixo de memória na string final.

Ambos os firmwares receptores contornam essa degradação aplicando uma varredura linear iterativa baseada na função `isprint()`. O algoritmo inspeciona cada índice do array de caracteres recebido e substitui qualquer byte corrompido ou invisível por um caractere de espaço em branco neutro (`' '`), impedindo a quebra de renderização na camada de aplicação:
```
void limparTexto(char* texto) {
  if (texto == NULL) return;
  for (int i = 0; i < strlen(texto); i++) {
    if (!isprint(texto[i])) {
      texto[i] = ' '; // Substitui lixo de memória por caractere neutro
    }
  }
}
```
### 5.3. Filtro de Redundância de Estado e Telemetria Delta (Nós Receptores)

Como o sinal de rádio varre o espectro continuamente, o mesmo pacote de dados é decodificado repetidamente pelo chip de rádio (múltiplas vezes por segundo). Para evitar o entupimento do canal de comunicação USB do host e quantificar o desempenho temporal do enlace, o loop dos receptores implementa um mecanismo de trava de estado associado a um cronômetro por hardware baseado em `millis()`:
```
// Verifica a presença de novos pacotes RDS prontos no hardware
if (rx.getRdsReady()) { 
  char* radioText = rx.getRdsText2A();

  if (radioText != NULL) {
    limparTexto(radioText);
    
    // FILTRO DE REDUNDÂNCIA: Avalia se a string atual difere do último estado registrado
    if (strlen(radioText) > 0 && strcmp(radioText, ultimaMensagem) != 0) {
      strcpy(ultimaMensagem, radioText); // Atualiza a variável de estado
      
      // --- TELEMETRIA DELTA CRONOMETRADA ---
      unsigned long tempoAtual = millis();
      unsigned long deltaChegada = tempoAtual - tempoUltimaChegada;
      tempoUltimaChegada = tempoAtual; // Armazena timestamp para o próximo ciclo

      // Emissão dos dados limpos agregados com métricas de tempo para o Java
      Serial.print("⏱️ ["); Serial.print(tempoAtual);
      Serial.print(" ms] (Delta: "); Serial.print(deltaChegada);
      Serial.print(" ms) | 📝 Mensagem Decodificada: [");
      Serial.print(ultimaMensagem); Serial.println("]");
    }
  }
}
```

Este filtro reduz drasticamente o uso de CPU da camada de processamento do computador Host, garantindo que o Orquestrador Java lide apenas com eventos de transição de dados válidos e strings previamente sanitizadas.

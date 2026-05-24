# 📚 Documentação Técnica: Sistema de Transmissão OOB via RDS

Esta documentação descreve a arquitetura, as lógicas de roteamento e os protocolos de comunicação desenvolvidos para o sistema de transmissão assíncrona utilizando a subportadora RDS (Radio Data System) em 57 kHz.

---

## 1. Visão Geral da Arquitetura

O ecossistema opera em um fluxo unidirecional (Simplex) ponta a ponta, dividido em três camadas principais:
1. **Camada de Aplicação (Java):** Interface de injeção de *payloads* e monitoramento.
2. **Camada de Enlace/Roteamento (ESP32/ESP8266):** Microcontroladores responsáveis por empacotar, sanitizar e transferir os dados via barramento I2C.
3. **Camada Física (RF - FM):** Transceptores de rádio (Si4713 e Si4703/RDA5807M) que modulam e demodulam o sinal no espectro de 106.1 MHz.

---

## 2. Protocolo de Handshake Ativo (Serial/USB)

Para permitir que o Orquestrador Java funcione como TX ou RX na mesma instância, foi desenvolvido um protocolo de identificação via porta Serial (115.200 bps).

### Fluxo de Reconhecimento:
1. O Java abre a porta serial e reinicia o microcontrolador via pulso DTR.
2. O Java envia a string de controle `PING_ID\n`.
3. O microcontrolador responde com seu identificador gravado em firmware:
   * Se responder `SYS_ID:TX_NODE`, o Java desbloqueia a interface de envio.
   * Se responder `SYS_ID:RX_NODE`, o Java bloqueia o envio e entra em escuta passiva.

---

## 3. Especificações do Firmware (C++)

### 3.1. Nó Transmissor (ESP32 + Si4713)
* **Hard Reset I2C:** Para evitar travamentos do barramento I2C nativos da arquitetura Espressif, o firmware realiza um ciclo de *Power Cycle* (LOW/HIGH) no pino RST do Si4713 antes da inicialização.
* **Padding Dinâmico:** O protocolo RDS exige blocos constantes para manter a subportadora estável. O firmware intercepta *strings* menores que 64 bytes e preenche dinamicamente o espaço residual com caracteres em branco (` `) até atingir o limite exato do registrador.

### 3.2. Nó Receptor (ESP8266 + Si4703/RDA5807M)
* **Vassoura Digital (Heurística de Ruído):** A demodulação FM está sujeita a alto nível de ruído eletromagnético (EMI), que injeta caracteres nulos ou corrompidos na memória. O firmware aplica a função `isprint()` do C++ iterando sobre o *buffer* bruto (64 bytes) para garantir que apenas blocos alfanuméricos válidos cheguem à memória lógica.
* **Filtro de Redundância:** O transceptor demodula a mesma *string* múltiplas vezes por segundo. O ESP8266 armazena o último estado válido e só realiza o tráfego USB (*Serial.println*) se o *hash* da nova *string* for diferente do estado anterior, zerando o *overhead* do Java.

---

## 4. Mapa de Hardware e Barramento

A comunicação interna entre os microcontroladores e os transceptores utiliza o protocolo I2C (Inter-Integrated Circuit). Os endereços I2C padrão são:
* **Si4713 (TX):** `0x11` ou `0x63`
* **Si4703 (RX):** `0x10`
* **RDA5807M (RX):** `0x10` e `0x11` (Acesso sequencial/aleatório)

### 4.1. Pinagem Crítica
A dependência de resistores de *pull-up* internos exige o uso de portas específicas na família ESP8266.
* **ESP32 (TX):** SDA no `D21` | SCL no `D22`
* **ESP8266 (RX):** SDA no `D4` | SCL no `D3` (Uso obrigatório para evitar Erro 127 de RSSI).

---

## 5. Limitações e Troubleshooting Crítico

* **Conflito de Bibliotecas I2C no Receptor:** A biblioteca `PU2CLR_SI470X` contém uma chamada `Wire.end()` na rotina de setup que conflita com a persistência do barramento no ESP8266. É estritamente necessário comentar essa linha no arquivo fonte `SI470X.cpp` para estabilidade em placas não-AVR.
* **Leitura de RSSI em 127 dBuV:** Indica que o barramento I2C do receptor está flutuando (aberto). Deve-se verificar a ordem dos pinos SDA/SCL e a integridade física dos *jumpers*.
* **Alcance e Antena:** O sistema em prototipação usa o aterramento do cabo P2 como antena passiva. Para distâncias superiores a 2,5 metros, recomenda-se a substituição do estágio final de RF por amplificadores e antenas dipolo ajustadas para o comprimento de onda ($\lambda$) correspondente a 106.1 MHz.

---
*Documentação mantida pela equipe de desenvolvimento para a reprodutibilidade metodológica do estudo de transmissão Out-of-Band via RDS.*

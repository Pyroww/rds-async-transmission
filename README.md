# rds-async-transmission
An open-source Out-of-Band (OOB) communication system for IoT/embedded devices using the 57 kHz FM subcarrier (RDS) via ESP32 and Java.

# 📻 Transmissão Assíncrona Out-of-Band via RDS para Sistemas Embarcados

Este repositório contém o código-fonte (software e firmware) e as instruções de replicação de hardware para o projeto de pesquisa **"Transmissão Assíncrona de Strings de Controle para Microcontroladores via RDS"**, submetido à ACM.

O sistema propõe uma infraestrutura de comunicação resiliente e de baixo custo, utilizando a subportadora de 57 kHz de rádio FM (RDS) para enviar cadeias de caracteres (strings) a dispositivos embarcados sem a necessidade de conectividade IP (Internet). 

Este repositório inclui a implementação do Orquestrador Java (Terminal de Injeção) e os firmwares contendo os mecanismos de estabilidade (*Padding* dinâmico) e filtragem heurística de ruído (*Vassoura Digital*).

---

## 🛠️ Requisitos de Hardware

Para replicar este experimento, você precisará dos seguintes componentes COTS (*Commercial Off-The-Shelf*):

**Nó Transmissor (TX):**
* 1x Microcontrolador ESP32
* 1x Transceptor FM Adafruit Si4713
* Fios de jumper (Jumpers)

**Nó Receptor (RX):**
* 1x Microcontrolador ESP8266 (NodeMCU)
* 1x Receptor FM SparkFun Si4703 (ou módulo equivalente suportado pela biblioteca SI470X)
* 1x Cabo de áudio P2 (utilizado passivamente na porta de fones para atuar como antena na malha de aterramento)

---

## 🔌 Esquema de Ligações (Pinagem)

### Ligações do Transmissor (ESP32 + Si4713)
| ESP32 Pin | Si4713 Pin | Função |
| :--- | :--- | :--- |
| 3V3 | VCC | Alimentação |
| GND | GND | Terra |
| GPIO 21 | SDA | Dados I2C |
| GPIO 22 | SCL | Clock I2C |
| GPIO 27 | RST | Reset de Hardware |

### Ligações do Receptor (ESP8266 + Si4703)
| ESP8266 Pin | Si4703 Pin | Função |
| :--- | :--- | :--- |
| 3V3 | 3.3V | Alimentação |
| GND | GND | Terra |
| D2 (GPIO 4) | SDIO | Dados I2C |
| D1 (GPIO 5) | SCLK | Clock I2C |
| D5 (GPIO 14)| RST | Reset de Hardware |

---

## 💻 Instalação e Execução

### Passo 1: Preparando o Hardware (Firmwares)
1. Abra a IDE do Arduino.
2. Certifique-se de instalar as seguintes bibliotecas através do Library Manager:
   * `Adafruit Si4713 Library` (Para o nó Transmissor)
   * `PU2CLR SI470X` (Para o nó Receptor)
3. Conecte o **ESP32**, abra o código presente na pasta `Firmware-TX`, compile e faça o upload.
4. Conecte o **ESP8266**, abra o código presente na pasta `Firmware-RX`, compile e faça o upload.

### Passo 2: O Orquestrador Java
O Orquestrador foi desenvolvido em Java com interface gráfica Swing e gerenciamento via Maven.
1. Instale o Java JDK 11 (ou superior) e o Maven em sua máquina.
2. Clone este repositório
3. Navegue até a pasta Java-Orquestrador.
4. O projeto utiliza a biblioteca jSerialComm para comunicação USB assíncrona. A dependência já está configurada no pom.xml.
5. Execute a aplicação via IDE (como VS Code, IntelliJ, Eclipse) ou linha de comando.

### Passo 3: Operando o Sistema
1. Alimente o ESP8266 (Receptor) via USB (apenas para energia) ou bateria. Abra o monitor serial para acompanhar a escuta passiva.

2. Conecte o ESP32 (Transmissor) na porta USB do seu computador principal.

3. Abra o Orquestrador Java, selecione a porta COM correspondente ao Transmissor e clique em "CONECTAR".

4. A interface realizará o Handshake Ativo, identificará o terminal e liberará o campo de texto para a injeção da string.

5. Digite um payload e envie. O dado será encapsulado em 64 bytes, transmitido em 106.1 MHz, recebido pelo nó remoto, filtrado pela Vassoura Digital e exibido de forma íntegra.

## 📖 Como Citar (Citation)
Se este projeto foi útil para a sua pesquisa, por favor, cite nosso artigo desenvolvido no Instituto Federal do Acre (IFAC):

Snippet de código
@inproceedings{Sa2026RDS,
  author = {Sá, André Lucas Soares da Silva de and Oliveira, Hugo Benjamim de and Miranda, Flávio},
  title = {Transmissão Assíncrona de Strings de Controle para Microcontroladores via RDS},
  year = {2026},
  publisher = {ACM},
  booktitle = {Proceedings of the ACM Multimedia Conference}
}

## 📄 Licença
Este projeto está licenciado sob a Licença MIT - veja o arquivo LICENSE para detalhes.

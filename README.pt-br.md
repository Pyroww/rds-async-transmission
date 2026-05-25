*Read this in other languages: [English](README.md)*
# rds-async-transmission
> *An open-source Out-of-Band (OOB) communication system for IoT/embedded devices using the 57 kHz FM subcarrier (RDS) via ESP32 and Java.*

# 📻 Transmissão Assíncrona Out-of-Band via RDS para Sistemas Embarcados

Este repositório contém o código-fonte (software e firmware) e as instruções de replicação de hardware para o projeto de pesquisa **"Transmissão Assíncrona de Strings de Controle para Microcontroladores via RDS"**, submetido à ACM.

O sistema propõe uma infraestrutura de comunicação resiliente e de baixo custo, utilizando a subportadora de 57 kHz de rádio FM (RDS) para enviar cadeias de caracteres (strings) a dispositivos embarcados sem a necessidade de conectividade IP (Internet). 

Este repositório inclui a implementação do Orquestrador Java (Terminal de Injeção) e os firmwares contendo os mecanismos de estabilidade (*Padding* dinâmico) e filtragem heurística de ruído (*Vassoura Digital*).

---

## 🛠️ Requisitos de Hardware

Para replicar este experimento, você precisará dos seguintes componentes COTS (*Commercial Off-The-Shelf*):

**Nó Transmissor (TX):**
* 1x Microcontrolador ESP32
* 1x Transmissor FM Adafruit Si4713
* Fios de jumper (Jumpers)

**Nó Receptor (RX):**
* 1x Microcontrolador ESP8266 (NodeMCU)
* 1x Receptor FM SparkFun Si4703 (ou módulo equivalente suportado pela biblioteca SI470X)
* 1x Cabo de áudio P2 (utilizado passivamente na porta de fones para atuar como antena na malha de aterramento)

---

## 🔌 Esquema de Ligações (Pinagem)
Importante ressaltar, que para melhor visualização, existe o diagrama ilustrativo no diretório `Data/`, contendo cada um das tabelas de esquemas abaixo

### Ligações do Transmissor (ESP32 + Si4713)
| ESP32 Pin | Si4713 Pin | Função |
| :--- | :--- | :--- |
| 3V3 | VIN | Alimentação |
| GND | GND | Terra |
| D21 | SDA | Dados I2C |
| D22 | SCL | Clock I2C |
| D27 | RST | Reset de Hardware |

### Ligações do Receptor (ESP8266 + Si4703)
| ESP8266 Pin | Si4703 Pin | Função |
| :--- | :--- | :--- |
| 3V3 | 3.3V | Alimentação |
| GND | GND | Terra |
| D2 | SDIO | Dados I2C |
| D1 | SCLK | Clock I2C |
| D5 | RST | Reset de Hardware |

### Receptor Secundario (ESP8266 + RDA 5807M)
| ESP8266 Pin | RDA 5807M Pin | Função |
| :--- | :--- | :--- |
| 3V3 | VIN | Alimentação |
| GND | GND | Terra |
| D3 | SCL | Dados I2C |
| D4 | SDA | Clock I2C |

---

## 💻 Instalação e Execução

### Passo 1: Preparando o Hardware (Firmwares)
1. **Instalação de Drivers USB:** Certifique-se de que o seu sistema operacional possui os drivers adequados para comunicação serial. O NodeMCU (ESP8266) comumente requer o driver [**CP210x USB to UART Bridge da Silicon Labs**](https://www.silabs.com/software-and-tools/usb-to-uart-bridge-vcp-drivers?tab=downloads), se caso o driver inicial não funcionar, instale o [driver CH340](https://sparks.gogo.co.nz/ch340.html)
2. Abra a IDE do Arduino.
3. Certifique-se de instalar as seguintes bibliotecas através do Library Manager:
   * `Adafruit Si4713 Library` (Para o nó Transmissor)
   * `PU2CLR SI470X` (Para o nó Receptor)
4. Conecte o **ESP32**, abra o código presente na pasta `Firmware-TX/Si4713`, compile e faça o upload.
5. Conecte o **ESP8266**, abra o código presente na pasta `Firmware-RX/Si4703`, compile e faça o upload.

### ⚠️ Nota de Compilação e Conflitos I2C (Troubleshooting)
Devido às diferenças arquitetônicas entre a família AVR tradicional e a família Espressif (ESP32/ESP8266), pode ocorrer um atropelamento na alocação dos pinos do barramento I2C por parte das bibliotecas originais. 

**Caso o transceptor não seja detectado no monitor serial (Erro no ESP8266 + SI470X):**
1. Navegue até a pasta de bibliotecas da IDE do Arduino (geralmente em `Documentos/Arduino/libraries/`).
2. Dentro da pasta `libraries/src`, procure pela pasta `PU2CLR_SI470X` Abra o arquivo fonte `SI470X.cpp` da biblioteca do receptor.
3. Localize a instrução `Wire.end();` dentro do método de inicialização/setup e **comente-a** (adicionando `//` no início da linha). 
4. Salve o arquivo e recompile. Isso impedirá que a biblioteca encerre o barramento prematuramente e a obrigará a respeitar os pinos (`D2` e `D1`) definidos pelo seu *firmware*.


### Passo 2: O Orquestrador Java
O Orquestrador foi desenvolvido em Java com interface gráfica Swing e gerenciamento via Maven.
1. Instale o Java JDK 23 (ou superior) e o Maven em sua máquina.
2. Faça o download do código-fonte deste repositório (via arquivo `.zip` disponibilizado pela plataforma de revisão) e extraia em sua máquina.
3. Navegue até a pasta Java-Orquestrador.
4. O projeto utiliza a biblioteca jSerialComm para comunicação USB assíncrona. A dependência já está configurada no pom.xml.
5. Execute a aplicação via IDE (como VS Code, IntelliJ, Eclipse) ou linha de comando.

### Passo 3: Operando o Sistema
1. Alimente o ESP8266 (Receptor) via USB (apenas para energia) ou bateria. execute o aplicativo em Java e se conecte a porta correspondente ao RX (receptor)

2. Conecte o ESP32 (Transmissor) na porta USB do seu computador principal.

3. Abra o Orquestrador Java, selecione a porta COM correspondente ao Transmissor e clique em "CONECTAR".

4. A interface realizará o Handshake Ativo, identificará o terminal (se é receptor ou se é Transmissor) e liberará o campo de texto para a injeção da string para o transmissor, do contrário, o app em java ativará o modo passivo de escuta RDS.

5. Digite um payload e envie. O dado será encapsulado em 64 bytes, transmitido em 106.1 MHz, recebido pelo nó remoto, filtrado pela Vassoura Digital e exibido de forma íntegra.

## 📄 Licença
Este projeto está licenciado sob a Licença GNU GPLv3 - veja o arquivo LICENSE para detalhes.

## 📖 Como Citar (Citation)
*(Nota: Informações de autoria e afiliação institucional foram suprimidas para garantir a integridade da revisão double-blind. A citação completa será disponibilizada na versão final (camera-ready) após o aceite).*

```bibtex
@inproceedings{Anonimo2026RDS,
  author = {Autores Omitidos para Revisão Duplo-Cego},
  title = {Transmissão Assíncrona de Strings de Controle para Microcontroladores via RDS},
  year = {2026},
  publisher = {ACM},
  booktitle = {Proceedings of the ACM Conference (Sob Revisão)}
}


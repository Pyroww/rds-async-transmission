# 📚 Documentação Técnica de Engenharia: Ecossistema de Comunicação Asíncrona OOB via RDS

Esta documentação técnica descreve a arquitetura de baixo nível, os protocolos de sinalização e os algoritmos lógicos implementados no ecossistema de comunicação *Out-of-Band* (OOB). O sistema viabiliza o transporte unidirecional assíncrono de cadeias de caracteres (*strings* de controle) utilizando a subportadora de 57 kHz de emissoras de rádio FM convencional por meio do protocolo RDS (Radio Data System).

---

## 1. Arquitetura Geral do Sistema e Camadas

O ecossistema é projetado sob um modelo de três camadas físicas e lógicas distintas:
1. **Camada de Aplicação e Orquestração (Java/Host):** Gerencia a injeção central de payloads e monitora os terminais de forma assíncrona a 115.200 bps.
2. **Camada de Enlace e Processamento Local (Microcontroladores):** Gateways baseados em ESP32 (Transmissor) e ESP8266 (Receptores) que executam rotinas lógicas de inicialização, barramento e sanitização de memória.
3. **Camada Física de Radiofrequência (RF):** Composta pelos transmissores e receptores integrados de rádio (Si4713 para modulação e Si4703 ou RDA5807M para demodulação) operando em onda portadora central de 106.1 MHz.

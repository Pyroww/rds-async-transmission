package com.dir.tcc;

import java.awt.Color;
import java.awt.FlowLayout;
import java.io.PrintWriter;

import javax.swing.JButton;
import javax.swing.JComboBox;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;
import javax.swing.JTextField;
import javax.swing.SwingUtilities;

import com.fazecast.jSerialComm.SerialPort;
import com.fazecast.jSerialComm.SerialPortDataListener;
import com.fazecast.jSerialComm.SerialPortEvent;

public class App {
    private static SerialPort portaEscolhida;
    private static PrintWriter output;
    
    private static StringBuilder bufferSerial = new StringBuilder();
    private static String modoAtual = "DESCONHECIDO"; 

    public static void main(String[] args) {
        JFrame frame = new JFrame("Command Center RDS Inteligente");
        frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        frame.setSize(550, 450);
        frame.setLayout(new FlowLayout(FlowLayout.CENTER, 10, 15));

        JLabel labelPorta = new JLabel("Dispositivo (Porta COM):");
        JComboBox<String> comboPortas = new JComboBox<>();
        for (SerialPort port : SerialPort.getCommPorts()) {
            comboPortas.addItem(port.getSystemPortName());
        }

        JButton botaoConectar = new JButton("1. CONECTAR E IDENTIFICAR");
        JButton botaoInjetar = new JButton("2. INJETAR SINAL");
        botaoInjetar.setEnabled(false); 

        JLabel labelMensagem = new JLabel("Payload para Injeção:");
        JTextField campoMensagem = new JTextField(25);
        campoMensagem.setEnabled(false); 

        JTextArea logArea = new JTextArea(15, 45);
        logArea.setEditable(false);
        logArea.setBackground(Color.DARK_GRAY);
        logArea.setForeground(Color.WHITE); 
        JScrollPane scroll = new JScrollPane(logArea);

        botaoConectar.addActionListener(e -> {
            String portaNome = (String) comboPortas.getSelectedItem();
            if (portaNome == null) return;

            portaEscolhida = SerialPort.getCommPort(portaNome);
            portaEscolhida.setBaudRate(115200);

            if (portaEscolhida.openPort()) {
                botaoConectar.setEnabled(false); comboPortas.setEnabled(false); bufferSerial.setLength(0);
                logArea.append("✅ Porta aberta. Interrogando placa USB...\n");

                
                portaEscolhida.clearDTR();
                portaEscolhida.clearRTS();

                output = new PrintWriter(portaEscolhida.getOutputStream());

                
                new Thread(() -> { 
                    try { 
                        Thread.sleep(2000); 
                    } catch (Exception ex) {} 
                    
                    output.print("PING_ID\n"); 
                    output.flush(); 
                }).start();

                portaEscolhida.addDataListener(new SerialPortDataListener() {
                    @Override
                    public int getListeningEvents() {
                        return SerialPort.LISTENING_EVENT_DATA_AVAILABLE;
                    }

                    @Override
                    public void serialEvent(SerialPortEvent event) {
                        if (event.getEventType() != SerialPort.LISTENING_EVENT_DATA_AVAILABLE) return;

                        byte[] newData = new byte[portaEscolhida.bytesAvailable()];
                        portaEscolhida.readBytes(newData, newData.length);
                        String stringRecebida = new String(newData);

                        bufferSerial.append(stringRecebida);

                        while (bufferSerial.indexOf("\n") != -1) {
                            int fimDaLinha = bufferSerial.indexOf("\n");
                            String linha = bufferSerial.substring(0, fimDaLinha).trim();
                            bufferSerial.delete(0, fimDaLinha + 1); 

                            if (linha.isEmpty()) continue;

                            SwingUtilities.invokeLater(() -> {
                                if (modoAtual.equals("DESCONHECIDO")) {
                                    if (linha.equals("SYS_ID:TRANSMISSOR")) {
                                        modoAtual = "TRANSMISSOR";
                                        frame.setTitle("Modo: TRANSMISSOR RDS (Si4713)");
                                        logArea.setBackground(Color.BLACK);
                                        logArea.setForeground(Color.GREEN);
                                        campoMensagem.setEnabled(true);
                                        botaoInjetar.setEnabled(true);
                                        logArea.append("🚀 TRANSMISSOR DETECTADO! Controles liberados.\n");
                                        logArea.append("--------------------------------------------------\n");
                                        return; 
                                    } else if (linha.equals("SYS_ID:RECEPTOR")) {
                                        modoAtual = "RECEPTOR";
                                        frame.setTitle("Modo: MONITOR RDS (NodeMCU)");
                                        logArea.setBackground(new Color(10, 10, 30));
                                        logArea.setForeground(Color.CYAN);
                                        logArea.append("🎧 RECEPTOR DETECTADO! Modo de escuta ativado.\n");
                                        logArea.append("--------------------------------------------------\n");
                                        return; 
                                    }
                                }

                                if (modoAtual.equals("TRANSMISSOR")) {
                                    logArea.append("ESP32: " + linha + "\n");
                                } else if (modoAtual.equals("RECEPTOR")) {
                                    logArea.append(linha + "\n"); 
                                } else {
                                    logArea.append("Log Raw: " + linha + "\n"); 
                                }
                                
                                logArea.setCaretPosition(logArea.getDocument().getLength());
                            });
                        }
                    }
                });

            } else {
                logArea.append("❌ ERRO: Não foi possível abrir a porta.\n");
            }
        });

        botaoInjetar.addActionListener(e -> {
            String msg = campoMensagem.getText().trim();
            if (msg.isEmpty()) return;

            output.print(msg + "\n");
            output.flush();
            
            logArea.append("🚀 JAVA INJETOU: [" + msg + "]\n");
            campoMensagem.setText(""); 
        });

        frame.add(labelPorta);
        frame.add(comboPortas);
        frame.add(botaoConectar);
        frame.add(labelMensagem);
        frame.add(campoMensagem);
        frame.add(botaoInjetar);
        frame.add(scroll);

        frame.setLocationRelativeTo(null);
        frame.setVisible(true);
    }
}

# 📡 Smart Radar Station — ESP32-S3 + Plataforma Web

Projeto completo de radar 180° com **Waveshare ESP32-S3 Zero N16R8**, **HC-SR04**, **SG90/MG90S**, **buzzer**, **LED**, **Wi-Fi**, **WebSocket**, **LittleFS** e dashboard responsivo. O mesmo painel funciona localmente em `192.168.4.1` e remotamente pela Vercel.

> O modo local não depende da internet. A ESP32 mantém o Access Point `SmartRadar` ativo mesmo quando também está conectada a uma rede Wi-Fi externa.

## 1. Sobre o projeto

O servo varre uma faixa configurável entre 0° e 180°. Em cada posição, o HC-SR04 mede a distância; o firmware processa a leitura, aplica filtro, avalia o alarme, registra o evento e transmite os dados ao navegador via WebSocket.

## 2. Funcionalidades

- Varredura 0° → 180° → 0° sem `delay()` bloqueante.
- HC-SR04 com timeout, leitura inválida e filtro de ruído.
- Alarme com histerese, buzzer pulsante e LED piscante.
- Wi-Fi AP + STA simultâneo.
- WebSocket local e remoto.
- Dashboard responsivo para Android, iPhone, tablet e PC.
- Canvas com grade, linha de varredura e persistência de obstáculos.
- Configuração em tempo real e persistência em NVS.
- Logs em ring buffer.
- Exportação CSV via WebSocket sem refresh.
- Watchdog, reconexão e telemetria de memória.
- Vercel com Redis opcional para backplane entre instâncias.

## 3. Componentes

| Componente | Quantidade |
|---|---:|
| ESP32-S3 Zero N16R8 | 1 |
| HC-SR04 | 1 |
| SG90 ou MG90S | 1 |
| Buzzer ativo | 1 |
| LED + resistor 220–330 Ω | 1 |
| Resistores 1 kΩ + 2 kΩ para ECHO | 1 par |
| Fonte 5 V para servo | 1 |

## 4. Esquema elétrico

```text
HC-SR04 TRIG  -> GPIO 4
HC-SR04 ECHO  -> divisor de tensão -> GPIO 5
SERVO SIGNAL  -> GPIO 6
BUZZER        -> GPIO 7
LED           -> GPIO 8
```

## 5. Tabela de GPIOs

| Função | GPIO |
|---|---:|
| TRIG | 4 |
| ECHO | 5 |
| Servo | 6 |
| Buzzer | 7 |
| LED | 8 |

Todos os GPIOs ficam em `src/Config.h`.

## 6. Alimentação

Alimente a ESP32 pela USB durante desenvolvimento. O servo deve usar uma fonte 5 V adequada, principalmente sob carga. Não use o pino 3,3 V da ESP32 para alimentar o servo.

```text
Fonte 5 V
  +5V ---- Servo VCC
  GND ---- Servo GND
    └---- ESP32 GND
```

O **GND deve ser comum** entre fonte, servo, sensor e ESP32.

## 7. Cuidados com 5 V / 3,3 V

O ECHO do HC-SR04 pode chegar a aproximadamente 5 V. O ESP32-S3 usa lógica 3,3 V. Use divisor resistivo ou level shifter.

Exemplo:

```text
HC-SR04 ECHO
    |
   1 kΩ
    |
    +---- GPIO 5 ESP32
    |
   2 kΩ
    |
   GND
```

Com 5 V na entrada, a saída fica próxima de 3,33 V.

## 8. Instalação do PlatformIO

1. Instale VS Code.
2. Instale a extensão **PlatformIO IDE**.
3. Reinicie o VS Code.

## 9. Como abrir o projeto

```bash
git clone https://github.com/eduardohenrique-dev2/RADARESP32MAISPLATWEB.git
cd RADARESP32MAISPLATWEB
code .
```

## 10. Como compilar

```bash
pio run
```

O projeto usa:

```ini
platform = espressif32 @ 7.0.1
board = esp32-s3-devkitm-1
framework = arduino
```

Para o N16R8 também estão configurados 16 MB Flash e PSRAM OPI.

## 11. Como fazer upload

```bash
pio run -t upload
```

Monitor serial:

```bash
pio device monitor
```

Saída esperada:

```text
================================
 SMART RADAR STATION
================================

WiFi iniciado
SSID: SmartRadar
IP: 192.168.4.1

Sistema pronto.
```

## 12. Como enviar arquivos para LittleFS

Depois do firmware:

```bash
pio run -t uploadfs
```

Arquivos enviados:

```text
data/index.html
data/style.css
data/app.js
data/radar.js
data/config.js
```

## 13. Como abrir o radar

Conecte à rede:

```text
SSID: SmartRadar
Senha: SmartRadar2026
```

Abra:

```text
http://192.168.4.1
```

## 14. Como conectar pelo celular

Sem internet: conecte diretamente ao `SmartRadar` e abra `192.168.4.1`.

Com acesso remoto: configure SSID/senha do Wi-Fi externo, host Vercel, Device ID e token no painel local. A ESP32 continuará oferecendo o AP local e, ao mesmo tempo, fará conexão de saída com a Vercel.

## 15. Como utilizar o dashboard

O dashboard mostra:

- ângulo e distância atuais;
- objetos detectados;
- menor/maior distância;
- leituras e alarmes;
- uptime;
- RSSI;
- IP do AP e IP da rede externa;
- clientes conectados ao AP;
- FPS;
- status do servo e sensor;
- heap e PSRAM;
- status da nuvem.

Controles: iniciar, parar, centralizar, reiniciar, ativar/desativar alarme, LED ON/OFF, limpar radar, exportar CSV e limpar logs.

## 16. Como exportar CSV

Clique em **Exportar CSV**. O navegador envia `export_logs` por WebSocket. A ESP32 responde com `csv_begin`, vários `csv_chunk` e `csv_end`. O JavaScript cria um Blob e inicia o download sem recarregar a página.

Cabeçalho:

```csv
Timestamp,Angulo,Distancia_cm,Alarme,Evento,RSSI
```

Nome do arquivo:

```text
radar_logs_YYYY-MM-DD_HH-MM-SS.csv
```

A exportação não apaga os logs.

## 17. Como limpar logs

Clique em **Limpar Logs**. O navegador exige confirmação:

```text
Tem certeza que deseja apagar todos os logs?
```

`Limpar Radar` remove somente os pontos visuais.

## 18. Como alterar configurações

Podem ser alterados sem recompilar:

- ângulo mínimo/máximo;
- passo angular;
- intervalo/velocidade do servo;
- distância máxima;
- distância do alarme;
- buzzer, LED e detecção;
- Wi-Fi externo;
- host Vercel;
- Device ID e token;
- ativação do acesso remoto.

GPIOs continuam em `src/Config.h`.

## 19. Solução de problemas

### `192.168.4.1` não abre

Grave o LittleFS:

```bash
pio run -t uploadfs
```

### Servo reinicia a ESP32

Use fonte 5 V externa para o servo e GND comum.

### `SENSOR ERROR`

Confira TRIG, ECHO, divisor de tensão, alimentação 5 V e posicionamento do HC-SR04.

### Wi-Fi SmartRadar não aparece

Abra o monitor serial e reinicie a placa.

### WebSocket local desconecta

Confirme que o celular continua na rede `SmartRadar`. Alguns celulares trocam automaticamente para dados móveis quando o Wi-Fi não tem internet.

### Painel Vercel conecta, mas dispositivo aparece offline

Confira se a ESP32 entrou no Wi-Fi externo, se o host Vercel está correto e se Device ID/token são iguais. Para uso com múltiplas instâncias Vercel, configure `REDIS_URL`.

## 20. Estrutura do código

```text
RADARESP32MAISPLATWEB/
├── api/
│   └── ws.js
├── data/
│   ├── index.html
│   ├── style.css
│   ├── app.js
│   ├── radar.js
│   └── config.js
├── src/
│   ├── main.cpp
│   ├── Config.h / Config.cpp
│   ├── Radar.h / Radar.cpp
│   ├── ServoControl.h / ServoControl.cpp
│   ├── Ultrasonic.h / Ultrasonic.cpp
│   ├── Alarm.h / Alarm.cpp
│   ├── Logger.h / Logger.cpp
│   ├── WebServer.h / WebServer.cpp
│   ├── WebSocket.h / WebSocket.cpp
│   └── WifiManager.h / WifiManager.cpp
├── platformio.ini
├── partitions.csv
├── package.json
├── vercel.json
└── README.md
```

## 21. Possíveis melhorias futuras

A arquitetura permite adicionar OTA, ESP32-CAM, IA, MQTT, Home Assistant, REST API, MicroSD, persistência de logs em LittleFS, banco de dados, aplicativo mobile, autenticação forte e múltiplos radares.

# 🌐 Deploy na Vercel

A Vercel disponibilizou suporte a WebSockets em Functions em Public Beta em 2026. Este repositório já contém `api/ws.js`, `package.json`, `vercel.json` e o script de build.

1. Importe este repositório na Vercel.
2. Faça o deploy na raiz do projeto.
3. O build executa `npm run build` e copia `data/` para `dist/`.
4. Abra o painel local da ESP32.
5. Configure o Wi-Fi externo.
6. Em **Host Vercel**, informe somente o domínio, por exemplo `meu-radar.vercel.app`.
7. Configure Device ID e token.
8. Ative a conexão remota.

A ESP32 se conecta para fora em:

```text
wss://SEU-PROJETO.vercel.app/api/ws
```

Não é necessário abrir portas no roteador.

## Redis opcional

Sem `REDIS_URL`, o relay funciona em memória da instância. Para maior confiabilidade quando navegador e ESP32 caem em instâncias Vercel diferentes, configure um Redis compatível e a variável:

```text
REDIS_URL
```

O código passa a usar Redis Pub/Sub automaticamente.

# 🧪 Testes recomendados

1. ESP32 inicia.
2. `SmartRadar` aparece.
3. Navegador conecta.
4. Servo gira.
5. HC-SR04 mede.
6. Radar aparece no Canvas.
7. Objeto próximo ativa alarme.
8. Buzzer/LED funcionam.
9. Logs são registrados.
10. CSV é baixado.
11. Limpeza de logs exige confirmação.
12. Painel Vercel funciona via 4G/5G.

# Protocolo WebSocket

Leitura:

```json
{"type":"radar","angle":90,"distance":52,"valid":true,"alarm":false,"timestamp":12345,"event":"SCAN","rssi":-42}
```

Comando:

```json
{"type":"command","command":"start"}
```

Comandos aceitos:

```text
start
stop
center
restart
alarm_on
alarm_off
led_on
led_off
clear_radar
clear_logs
export_logs
get_config
```

# Segurança

O token padrão é somente para primeiro teste:

```text
smart-radar-demo-2026
```

Troque-o antes de demonstrações públicas. O acesso remoto usa `wss://`, porém Device ID + token é autenticação básica. No firmware atual, `beginSSL()` é usado sem CA/fingerprint, então a conexão é criptografada, mas a validação do certificado pelo ESP32 não é forte o suficiente para produção. Para produção, configure CA/certificate bundle e autenticação mais robusta.

# Referências

- Waveshare ESP32-S3 Zero: https://www.waveshare.com/wiki/ESP32-S3-Zero
- PlatformIO Espressif32: https://docs.platformio.org/en/latest/platforms/espressif32.html
- ESPAsyncWebServer: https://registry.platformio.org/libraries/esp32async/ESPAsyncWebServer
- Vercel WebSocket Public Beta: https://vercel.com/changelog/websocket-support-is-now-in-public-beta

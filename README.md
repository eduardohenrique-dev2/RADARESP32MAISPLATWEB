# 📡 Smart Radar Station — ESP32-S3 + Plataforma Web

Projeto completo de radar 180° com **Waveshare ESP32-S3 Zero N16R8**, **HC-SR04**, **SG90/MG90S**, **joystick HW-504**, **buzzer**, **RGB WS2812 onboard**, **Wi-Fi**, **WebSocket**, **LittleFS** e dashboard responsivo. O mesmo painel funciona localmente em `192.168.4.1` e remotamente pela Vercel.

> O modo local não depende da internet. A ESP32 mantém o Access Point `SmartRadar` ativo mesmo quando também está conectada a uma rede Wi-Fi externa.

## 1. Sobre o projeto

O servo varre uma faixa configurável entre 0° e 180°. Em cada posição, o HC-SR04 mede a distância; o firmware processa a leitura, aplica filtro, avalia o alarme, registra o evento e transmite os dados ao navegador via WebSocket.

O joystick HW-504 adiciona controle físico: o botão `SW` alterna entre **AUTOMÁTICO** e **MANUAL**. Em modo manual, `VRX` movimenta o servo para esquerda/direita e o HC-SR04 continua medindo no ângulo escolhido. `VRY` já está conectado e lido pelo firmware, reservado para expansão futura, como segundo servo ou ajuste proporcional.

## 2. Funcionalidades

- Varredura 0° → 180° → 0° sem `delay()` bloqueante.
- Controle manual físico com joystick HW-504.
- Botão do joystick alternando AUTO ↔ MANUAL.
- HC-SR04 com timeout, leitura inválida e filtro de ruído.
- Alarme com histerese e buzzer pulsante.
- RGB WS2812 onboard: verde no comando LED ON e vermelho piscante no alarme.
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
| Joystick HW-504 | 1 |
| Buzzer ativo | 1 |
| RGB WS2812 onboard | já integrado na ESP32 |
| Resistores 1 kΩ + 2 kΩ para ECHO | 1 par |
| Fonte 5 V para servo | 1 |

## 4. Esquema elétrico

```text
ESP32-S3 ZERO

HC-SR04 TRIG  -> GPIO 4
HC-SR04 ECHO  -> divisor de tensão -> GPIO 5
SERVO SIGNAL  -> GPIO 6
BUZZER        -> GPIO 7
RGB WS2812    -> GPIO 21 (já integrado na placa)

JOYSTICK HW-504
GND           -> GND
+5V           -> 3V3 da ESP32  <- IMPORTANTE: usar 3,3 V neste projeto
VRX           -> GPIO 1
VRY           -> GPIO 2
SW            -> GPIO 8
```

Apesar de o pino do módulo HW-504 vir marcado como `+5V`, o joystick pode funcionar em 3,3 V. Neste projeto ele deve ser alimentado em **3V3**, para que `VRX` e `VRY` não apresentem tensão analógica acima do limite do ESP32-S3.

## 5. Tabela de GPIOs

| Função | GPIO | Tipo |
|---|---:|---|
| Joystick VRX | 1 | ADC |
| Joystick VRY | 2 | ADC |
| HC-SR04 TRIG | 4 | Digital OUT |
| HC-SR04 ECHO | 5 | Digital IN via divisor |
| Servo | 6 | PWM |
| Buzzer | 7 | Digital OUT |
| Joystick SW | 8 | Digital IN / Pull-up |
| RGB WS2812 onboard | 21 | Digital / NeoPixel |

Todos os GPIOs ficam centralizados em `src/Config.h`.

## 6. Alimentação

Alimente a ESP32 pela USB durante desenvolvimento. O servo deve usar uma fonte 5 V adequada, principalmente sob carga. Não use o pino 3,3 V da ESP32 para alimentar o servo.

```text
Fonte 5 V externa
  +5V ---------------- Servo VCC
  GND ---------------- Servo GND
    |
    +----------------- ESP32 GND
    +----------------- HC-SR04 GND

ESP32 3V3 ------------ HW-504 +5V (pino de alimentação do módulo)
ESP32 GND ------------ HW-504 GND
```

O **GND deve ser comum** entre fonte, servo, sensor, joystick e ESP32.

## 7. Cuidados com 5 V / 3,3 V

### HC-SR04

O ECHO do HC-SR04 pode chegar a aproximadamente 5 V. O ESP32-S3 usa lógica 3,3 V. Use divisor resistivo ou level shifter.

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

### Joystick HW-504

Não alimente o HW-504 em 5 V quando `VRX` e `VRY` estiverem ligados diretamente ao ADC da ESP32. Use:

```text
HW-504 +5V -> ESP32 3V3
```

Assim os sinais analógicos permanecem aproximadamente entre 0 V e 3,3 V.

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

Se o projeto já estiver clonado:

```bash
git pull
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

Saída esperada inclui:

```text
WiFi iniciado
SSID: SmartRadar
IP: 192.168.4.1
[Joystick] HW-504 pronto
```

Na ESP32-S3-Zero pode ser necessário segurar **BOOT** enquanto conecta o USB para entrar no modo de gravação.

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

## 15. Controle pelo joystick

Ao ligar, o radar inicia em modo automático.

```text
AUTO
0° -> 180° -> 0°
```

Pressione o joystick (`SW`) uma vez:

```text
MANUAL
```

No modo manual:

```text
VRX para esquerda  -> servo gira para um lado
VRX no centro      -> servo mantém o ângulo
VRX para direita   -> servo gira para o outro lado
```

O movimento é proporcional: quanto mais o joystick for deslocado, maior o passo de movimento.

O sensor continua medindo, gerando alarmes, logs e dados para o dashboard.

Pressione `SW` novamente para voltar à varredura automática.

`VRY` está reservado no firmware para expansão futura.

## 16. Como utilizar o dashboard

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

Controles: iniciar, parar, centralizar, reiniciar, ativar/desativar alarme, RGB ON/OFF, limpar radar, exportar CSV e limpar logs.

## 17. Como exportar CSV

Clique em **Exportar CSV**. O navegador envia `export_logs` por WebSocket. A ESP32 responde com `csv_begin`, vários `csv_chunk` e `csv_end`. O JavaScript cria um Blob e inicia o download sem recarregar a página.

Cabeçalho:

```csv
Timestamp,Angulo,Distancia_cm,Alarme,Evento,RSSI
```

Nome do arquivo:

```text
radar_logs_YYYY-MM-DD_HH-MM-SS.csv
```

Em modo manual, leituras comuns são registradas como `MANUAL_SCAN`.

A exportação não apaga os logs.

## 18. Como limpar logs

Clique em **Limpar Logs**. O navegador exige confirmação:

```text
Tem certeza que deseja apagar todos os logs?
```

`Limpar Radar` remove somente os pontos visuais.

## 19. Como alterar configurações

Podem ser alterados sem recompilar:

- ângulo mínimo/máximo;
- passo angular;
- intervalo/velocidade do servo;
- distância máxima;
- distância do alarme;
- buzzer, RGB e detecção;
- Wi-Fi externo;
- host Vercel;
- Device ID e token;
- ativação do acesso remoto.

GPIOs continuam em `src/Config.h`.

## 20. Solução de problemas

### Joystick vai para o lado errado

Inverta a lógica de `VRX` em `src/JoystickControl.cpp` ou fisicamente gire o módulo 180° na montagem.

### Joystick fica mexendo sozinho

Aumente `JOYSTICK_DEADZONE` em `src/Config.h`. O padrão já inclui uma zona morta ao redor do centro.

### Botão não funciona

O `SW` usa `INPUT_PULLUP`: solto = HIGH, pressionado = LOW. Confira `SW -> GPIO 8` e `GND` comum.

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

## 21. Estrutura do código

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
│   ├── JoystickControl.h / JoystickControl.cpp
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

## 22. Possíveis melhorias futuras

A arquitetura permite adicionar segundo servo usando `VRY`, OTA, ESP32-CAM, IA, MQTT, Home Assistant, REST API, MicroSD, persistência de logs em LittleFS, banco de dados, aplicativo mobile, autenticação forte e múltiplos radares.

# 🌐 Deploy na Vercel

Este repositório contém `api/ws.js`, `package.json`, `vercel.json` e script de build para a plataforma web.

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

Sem `REDIS_URL`, o relay funciona em memória da instância. Para maior confiabilidade quando navegador e ESP32 caem em instâncias diferentes, configure um Redis compatível e a variável:

```text
REDIS_URL
```

# 🧪 Testes recomendados

1. ESP32 inicia.
2. RGB onboard responde.
3. HW-504 é reconhecido no Serial.
4. `SW` alterna AUTO ↔ MANUAL.
5. `VRX` movimenta o servo em manual.
6. `SmartRadar` aparece.
7. Navegador conecta.
8. Servo gira automaticamente.
9. HC-SR04 mede.
10. Radar aparece no Canvas.
11. Objeto próximo ativa alarme.
12. Buzzer e RGB vermelho piscante funcionam.
13. Logs são registrados.
14. CSV é baixado.
15. Limpeza de logs exige confirmação.
16. Painel Vercel funciona via rede externa.

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

Troque-o antes de demonstrações públicas.

# Referências

- Waveshare ESP32-S3 Zero: https://www.waveshare.com/wiki/ESP32-S3-Zero
- PlatformIO Espressif32: https://docs.platformio.org/en/latest/platforms/espressif32.html
- ESPAsyncWebServer: https://registry.platformio.org/libraries/esp32async/ESPAsyncWebServer
- Adafruit NeoPixel: https://registry.platformio.org/libraries/adafruit/Adafruit%20NeoPixel

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiUdp.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

const char* ssid = "WiFi_Name";
const char* password = "WiFi_Password";

//  ipconfig 🔴
const char* pcIP = "192.168.xx.xx";   //IP  
unsigned int localPort = 4210;

WiFiUDP Udp;
ESP8266WebServer server(80);

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// 
#define BTN_UP    D5
#define BTN_OK    D6
#define BTN_DOWN  D7

//Ultrasonic
#define TRIG_PIN  D3
#define ECHO_PIN  D4
long duration;
int distance;
bool personDetected = false;       // 
unsigned long lastSonarTime = 0;   // 

const char* menuItems[] = {
  "Fire Mode", 
  "Forest Mode", 
  "Xmas Mode", 
  "Sea Mode", 
  "Space Mode", 
  "TURN OFF"
};
int menuIndex = 0;
int maxMenus = 6;
String currentMode = "OFF";

// 
const char page[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <meta charset="UTF-8">
  <title>ESP8266 Lighting Control Pro</title>
  <style>
    * { margin: 0; padding: 0; box-sizing: border-box; }
    
    body {
      font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
      background: linear-gradient(135deg, #1e3c72 0%, #2a5298 50%, #7e22ce 100%);
      min-height: 100vh;
      display: flex;
      justify-content: center;
      align-items: center;
      padding: 20px;
      overflow-x: hidden;
    }
    
    /* Animated background particles */
    .bg-animation {
      position: fixed;
      top: 0;
      left: 0;
      width: 100%;
      height: 100%;
      overflow: hidden;
      z-index: 0;
      pointer-events: none;
    }
    
    .particle {
      position: absolute;
      background: rgba(255, 255, 255, 0.3);
      border-radius: 50%;
      animation: float 15s infinite ease-in-out;
    }
    
    @keyframes float {
      0%, 100% { transform: translateY(0) translateX(0) scale(1); opacity: 0; }
      10% { opacity: 1; }
      90% { opacity: 1; }
      100% { transform: translateY(-100vh) translateX(50px) scale(0.5); opacity: 0; }
    }
    
    .container {
      position: relative;
      z-index: 1;
      background: rgba(255, 255, 255, 0.98);
      border-radius: 35px;
      padding: 45px 35px;
      box-shadow: 0 25px 70px rgba(0,0,0,0.4), 0 0 100px rgba(126, 34, 206, 0.3);
      max-width: 480px;
      width: 100%;
      animation: slideIn 0.6s cubic-bezier(0.68, -0.55, 0.265, 1.55);
      backdrop-filter: blur(10px);
    }
    
    @keyframes slideIn {
      from { opacity: 0; transform: translateY(50px) scale(0.9); }
      to { opacity: 1; transform: translateY(0) scale(1); }
    }
    
    .header {
      text-align: center;
      margin-bottom: 35px;
    }
    
    h1 {
      color: #1a202c;
      font-size: 32px;
      font-weight: 800;
      margin-bottom: 8px;
      background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
      -webkit-background-clip: text;
      -webkit-text-fill-color: transparent;
      background-clip: text;
    }
    
    .subtitle {
      color: #718096;
      font-size: 14px;
      font-weight: 500;
      letter-spacing: 0.5px;
    }
    
    .status-card {
      background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
      border-radius: 25px;
      padding: 25px;
      margin-bottom: 35px;
      text-align: center;
      color: white;
      box-shadow: 0 15px 35px rgba(102, 126, 234, 0.5);
      position: relative;
      overflow: hidden;
    }
    
    .status-card::before {
      content: '';
      position: absolute;
      top: -50%;
      left: -50%;
      width: 200%;
      height: 200%;
      background: linear-gradient(45deg, transparent, rgba(255,255,255,0.1), transparent);
      transform: rotate(45deg);
      animation: shine 3s infinite;
    }
    
    @keyframes shine {
      0% { transform: translateX(-100%) translateY(-100%) rotate(45deg); }
      100% { transform: translateX(100%) translateY(100%) rotate(45deg); }
    }
    
    .status-label {
      font-size: 13px;
      opacity: 0.95;
      text-transform: uppercase;
      letter-spacing: 2px;
      font-weight: 600;
      position: relative;
    }
    
    #currentMode {
      font-size: 28px;
      font-weight: 800;
      margin-top: 8px;
      animation: pulse 2.5s ease-in-out infinite;
      position: relative;
      text-shadow: 0 2px 10px rgba(0,0,0,0.2);
    }
    
    @keyframes pulse {
      0%, 100% { opacity: 1; transform: scale(1); }
      50% { opacity: 0.85; transform: scale(1.03); }
    }
    
    .button-grid {
      display: grid;
      gap: 18px;
    }
    
    button {
      position: relative;
      border: none;
      border-radius: 18px;
      padding: 22px;
      font-size: 19px;
      font-weight: 700;
      color: white;
      cursor: pointer;
      transition: all 0.4s cubic-bezier(0.175, 0.885, 0.32, 1.275);
      overflow: hidden;
      box-shadow: 0 6px 20px rgba(0,0,0,0.25);
      transform-style: preserve-3d;
    }
    
    button::before {
      content: '';
      position: absolute;
      top: 50%;
      left: 50%;
      width: 0;
      height: 0;
      border-radius: 50%;
      background: rgba(255,255,255,0.4);
      transform: translate(-50%, -50%);
      transition: width 0.7s, height 0.7s;
    }
    
    button:active::before {
      width: 400px;
      height: 400px;
    }
    
    button:hover {
      transform: translateY(-5px) scale(1.02);
      box-shadow: 0 12px 35px rgba(0,0,0,0.35);
    }
    
    button:active {
      transform: translateY(-2px) scale(0.98);
    }
    
    .fire {
      background: linear-gradient(135deg, #ff6b6b 0%, #ee5a6f 50%, #ff4757 100%);
    }
    
    .fire:hover {
      background: linear-gradient(135deg, #ff8787 0%, #ff6b7a 50%, #ff5e6e 100%);
      box-shadow: 0 12px 35px rgba(238, 90, 111, 0.6);
    }
    
    .forest {
      background: linear-gradient(135deg, #56ab2f 0%, #7cb342 50%, #a8e063 100%);
    }
    
    .forest:hover {
      background: linear-gradient(135deg, #6bc03d 0%, #8fc653 50%, #b5e876 100%);
      box-shadow: 0 12px 35px rgba(124, 179, 66, 0.6);
    }
    
    .xmas {
      background: linear-gradient(135deg, #d31027 0%, #ea384d 25%, #45b649 75%, #2ecc71 100%);
      animation: xmasGlow 2.5s ease-in-out infinite;
    }
    
    @keyframes xmasGlow {
      0%, 100% { 
        box-shadow: 0 6px 20px rgba(211, 16, 39, 0.6);
        filter: brightness(1);
      }
      50% { 
        box-shadow: 0 6px 30px rgba(69, 182, 73, 0.8);
        filter: brightness(1.1);
      }
    }
    
    .sea {
      background: linear-gradient(135deg, #2980b9 0%, #3498db 50%, #5dade2 100%);
      position: relative;
    }
    
    .sea::after {
      content: '';
      position: absolute;
      top: 0;
      left: -100%;
      width: 100%;
      height: 100%;
      background: linear-gradient(90deg, transparent, rgba(255,255,255,0.3), transparent);
      animation: wave 2s infinite;
    }
    
    @keyframes wave {
      0% { left: -100%; }
      100% { left: 200%; }
    }
    
    .sea:hover {
      background: linear-gradient(135deg, #3498db 0%, #5dade2 50%, #85c1e9 100%);
      box-shadow: 0 12px 35px rgba(52, 152, 219, 0.6);
    }
    
    .space {
      background: linear-gradient(135deg, #0f2027 0%, #203a43 50%, #2c5364 100%);
      border: 2px solid rgba(255,255,255,0.3);
      position: relative;
      overflow: hidden;
    }
    
    .space::before {
      content: '✦';
      position: absolute;
      color: white;
      font-size: 12px;
      animation: twinkle 1.5s infinite;
      opacity: 0;
    }
    
    @keyframes twinkle {
      0%, 100% { opacity: 0; }
      50% { opacity: 1; }
    }
    
    .space:hover {
      background: linear-gradient(135deg, #203a43 0%, #2c5364 50%, #395a6b 100%);
      box-shadow: 0 12px 35px rgba(44, 83, 100, 0.6);
    }
    
    .off {
      background: linear-gradient(135deg, #434343 0%, #2c2c2c 50%, #000000 100%);
    }
    
    .off:hover {
      background: linear-gradient(135deg, #555555 0%, #3e3e3e 50%, #1a1a1a 100%);
      box-shadow: 0 12px 35px rgba(67, 67, 67, 0.6);
    }
    
    .icon {
      font-size: 32px;
      margin-right: 12px;
      display: inline-block;
      animation: bounce 2.5s ease-in-out infinite;
      filter: drop-shadow(0 2px 5px rgba(0,0,0,0.3));
    }
    
    @keyframes bounce {
      0%, 100% { transform: translateY(0); }
      50% { transform: translateY(-8px); }
    }
    
    button:hover .icon {
      animation: spin 0.6s ease-in-out;
    }
    
    @keyframes spin {
      from { transform: rotate(0deg) scale(1); }
      50% { transform: rotate(180deg) scale(1.2); }
      to { transform: rotate(360deg) scale(1); }
    }
    
    .loader {
      display: none;
      width: 24px;
      height: 24px;
      border: 4px solid rgba(102, 126, 234, 0.3);
      border-top-color: #667eea;
      border-radius: 50%;
      animation: spin-loader 0.8s linear infinite;
      margin: 15px auto 0;
    }
    
    @keyframes spin-loader {
      to { transform: rotate(360deg); }
    }
    
    .success-msg {
      background: linear-gradient(135deg, #48bb78 0%, #38a169 100%);
      color: white;
      padding: 18px;
      border-radius: 15px;
      margin-top: 25px;
      text-align: center;
      display: none;
      animation: slideUp 0.4s ease-out;
      box-shadow: 0 10px 25px rgba(72, 187, 120, 0.4);
      font-weight: 600;
    }
    
    @keyframes slideUp {
      from { opacity: 0; transform: translateY(20px); }
      to { opacity: 1; transform: translateY(0); }
    }
    
    .footer {
      text-align: center;
      margin-top: 35px;
      color: #a0aec0;
      font-size: 13px;
      font-weight: 500;
    }
    
    .footer span {
      display: inline-block;
      animation: fadeIn 2s ease-in;
    }
    
    @keyframes fadeIn {
      from { opacity: 0; }
      to { opacity: 1; }
    }
  </style>
</head>
<body>
  <div class="bg-animation">
    <div class="particle" style="width:8px;height:8px;left:10%;animation-delay:0s;"></div>
    <div class="particle" style="width:12px;height:12px;left:25%;animation-delay:2s;"></div>
    <div class="particle" style="width:6px;height:6px;left:40%;animation-delay:4s;"></div>
    <div class="particle" style="width:10px;height:10px;left:55%;animation-delay:1s;"></div>
    <div class="particle" style="width:8px;height:8px;left:70%;animation-delay:3s;"></div>
    <div class="particle" style="width:14px;height:14px;left:85%;animation-delay:5s;"></div>
  </div>

  <div class="container">
    <div class="header">
      <h1>🎮 ESP Lighting Control</h1>
      <p class="subtitle">Professional Edition v2.0</p>
    </div>
    
    <div class="status-card">
      <div class="status-label">Current Mode</div>
      <div id="currentMode">Loading...</div>
    </div>
    
    <div class="button-grid">
      <button class="fire" onclick="sendCommand('fire', 'FIRE', '🔥 Fire Mode')">
        <span class="icon">🔥</span>Fire Mode
      </button>
      
      <button class="forest" onclick="sendCommand('forest', 'FOREST', '🌲 Forest Mode')">
        <span class="icon">🌲</span>Forest Mode
      </button>
      
      <button class="xmas" onclick="sendCommand('xmas', 'XMAS', '🎄 Xmas Mode')">
        <span class="icon">🎄</span>Xmas Mode
      </button>
      
      <button class="sea" onclick="sendCommand('sea', 'SEA', '🌊 Sea Mode')">
        <span class="icon">🌊</span>Sea Mode
      </button>
      
      <button class="space" onclick="sendCommand('space', 'SPACE', '🌌 Space Mode')">
        <span class="icon">🌌</span>Space Mode
      </button>
      
      <button class="off" onclick="sendCommand('off', 'OFF', '⚫ OFF')">
        <span class="icon">⚫</span>TURN OFF
      </button>
    </div>
    
    <div class="loader" id="loader"></div>
    <div class="success-msg" id="successMsg">✓ Command Sent Successfully!</div>
    
    <div class="footer">
      <span>⚡ Designed by Eng.Ahmed</span>
    </div>
  </div>
  
  <script>
    function updateStatus() {
      fetch('/status')
        .then(r => r.text())
        .then(mode => {
          document.getElementById('currentMode').textContent = mode;
        })
        .catch(() => {
          document.getElementById('currentMode').textContent = 'Connection Error';
        });
    }
    
    function sendCommand(endpoint, cmd, displayName) {
      const loader = document.getElementById('loader');
      const successMsg = document.getElementById('successMsg');
      
      loader.style.display = 'block';
      
      fetch('/' + endpoint)
        .then(response => {
          if(response.ok) {
            loader.style.display = 'none';
            document.getElementById('currentMode').textContent = displayName;
            
            successMsg.style.display = 'block';
            setTimeout(() => {
              successMsg.style.display = 'none';
            }, 2500);
          }
        })
        .catch(err => {
          loader.style.display = 'none';
          alert('Connection error! Check your WiFi.');
        });
    }
    
    updateStatus();
    setInterval(updateStatus, 5000);
  </script>
</body>
</html>
)rawliteral";

void sendCommand(String cmd, String modeName) {
  currentMode = modeName;
  Serial.println(cmd); 
  Udp.beginPacket(pcIP, localPort);
  Udp.write(cmd.c_str());
  Udp.endPacket();

  display.clearDisplay();
  display.setCursor(0, 25);
  display.setTextSize(2);
  display.println(modeName);
  display.display();
  delay(500);
  drawMenu();
}

void handleRoot() { server.send(200, "text/html", page); }
void handleStatus() { server.send(200, "text/plain", currentMode); }
void handleFire() { sendCommand("FIRE", "Fire Mode"); server.send(200, "text/plain", "OK"); }
void handleForest() { sendCommand("FOREST", "Forest Mode"); server.send(200, "text/plain", "OK"); }
void handleXmas() { sendCommand("XMAS", "Xmas Mode"); server.send(200, "text/plain", "OK"); }
void handleSea() { sendCommand("SEA", "Sea Mode"); server.send(200, "text/plain", "OK"); }
void handleSpace() { sendCommand("SPACE", "Space Mode"); server.send(200, "text/plain", "OK"); }
void handleOff() { sendCommand("OFF", "TURN OFF"); server.send(200, "text/plain", "OK"); }

void setup() {
  pinMode(BTN_UP, INPUT_PULLUP);
  pinMode(BTN_OK, INPUT_PULLUP);
  pinMode(BTN_DOWN, INPUT_PULLUP);

  // 
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  Serial.begin(9600);
  Wire.begin(D2, D1);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println("Connecting WiFi...");
  display.display();

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  
  server.on("/", handleRoot);
  server.on("/status", handleStatus);
  server.on("/fire", handleFire);
  server.on("/forest", handleForest);
  server.on("/xmas", handleXmas);
  server.on("/sea", handleSea);
  server.on("/space", handleSpace);
  server.on("/off", handleOff);
  server.begin();

  currentMode = "OFF";
  drawMenu();
}

void loop() {
  server.handleClient();
  
  // 
  // 
  if (millis() - lastSonarTime > 500) {
    lastSonarTime = millis();
    
    //
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);
    
    //
    duration = pulseIn(ECHO_PIN, HIGH);
    distance = duration * 0.034 / 2;
    
    //:
    if (distance > 0 && distance < 50) {
      if (!personDetected) {
        Serial.println("BOSS_DETECTED"); // Debug
        
        // 
        Udp.beginPacket(pcIP, localPort);
        Udp.write("BOSS_DETECTED");
        Udp.endPacket();
        
        personDetected = true; //    
      }
    } 
    //
    else if (distance > 60 || distance == 0) {
      personDetected = false;
    }
  }

  //
  if(digitalRead(BTN_UP)==LOW) { menuIndex--; if(menuIndex<0) menuIndex=maxMenus-1; drawMenu(); delay(200); }
  if(digitalRead(BTN_DOWN)==LOW) { menuIndex++; if(menuIndex>=maxMenus) menuIndex=0; drawMenu(); delay(200); }
  if(digitalRead(BTN_OK)==LOW) {
    if(menuIndex==0) sendCommand("FIRE", "Fire Mode");
    else if(menuIndex==1) sendCommand("FOREST", "Forest Mode");
    else if(menuIndex==2) sendCommand("XMAS", "Xmas Mode");
    else if(menuIndex==3) sendCommand("SEA", "Sea Mode");
    else if(menuIndex==4) sendCommand("SPACE", "Space Mode");
    else if(menuIndex==5) sendCommand("OFF", "TURN OFF");
  }
}

void drawMenu() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println(WiFi.localIP());
  display.drawLine(0, 10, 128, 10, WHITE);

  int start = menuIndex - 1;
  if (start < 0) start = 0;
  if (start > maxMenus - 3) start = maxMenus - 3;
  if (start < 0) start = 0;

  for(int i=0; i<3; i++) {
    int idx = start + i;
    if(idx >= maxMenus) break;
    int y = 18 + (i * 15);
    if(idx == menuIndex) {
      display.fillRect(0, y-2, 128, 14, WHITE);
      display.setTextColor(BLACK);
      display.setCursor(5, y);
      display.print("> ");
      display.println(menuItems[idx]);
    } else {
      display.setTextColor(WHITE);
      display.setCursor(5, y);
      display.print("  ");
      display.println(menuItems[idx]);
    }
  }
  display.display();
}
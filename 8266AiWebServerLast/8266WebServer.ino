#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// 
const char* ssid = " WIFI NAME"; // 
const char* password = "PASSWORD";            


#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);


#define BTN_UP    D5
#define BTN_OK    D6
#define BTN_DOWN  D7


ESP8266WebServer server(80);


const char* menuItems[] = {"Fire Mode", "Forest Mode", "Xmas Mode", "TURN OFF"};
int menuIndex = 0;
int maxMenus = 4;

// =====  (HTML) =====
const char page[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    body { font-family: Arial; text-align: center; background-color: #000; color: white; margin-top: 50px;}
    h1 { color: #ff9800; }
    button { display: block; width: 80%; margin: 15px auto; padding: 15px; font-size: 20px; border: none; border-radius: 10px; cursor: pointer; color: white; font-weight: bold;}
    .fire { background-color: #ff4500; }
    .forest { background-color: #228b22; }
    .xmas { background-color: #c71585; }
    .off { background-color: #333; }
  </style>
</head>
<body>
  <h1>ESP Lighting Control</h1>
  <button class="fire" onclick="fetch('/fire')">🔥 Fire Mode</button>
  <button class="forest" onclick="fetch('/forest')">🌲 Forest Mode</button>
  <button class="xmas" onclick="fetch('/xmas')">🌈 Xmas Mode</button>
  <button class="off" onclick="fetch('/off')">⚫ TURN OFF</button>
  <p id="status">Status: Ready</p>
  
  <script>
    // دالة لتحديث الحالة عند الضغط
    function update(txt) { document.getElementById("status").innerHTML = "Status: " + txt; }
  </script>
</body>
</html>
)rawliteral";

void sendCommand(String cmd, String modeName) {
  // .  ESP32  
  Serial.println(cmd); 
  
  // 2.  
  display.clearDisplay();
  display.setCursor(0, 20);
  display.setTextSize(2);
  display.println(modeName);
  display.setTextSize(1);
  display.setCursor(0, 50);
  display.println("Command Sent!");
  display.display();
  
  
  delay(500);
  drawMenu();
}

/
void handleRoot() { server.send(200, "text/html", page); }
void handleFire() { sendCommand("FIRE", "Fire Mode"); server.send(200, "text/plain", "OK"); }
void handleForest() { sendCommand("FOREST", "Forest Mode"); server.send(200, "text/plain", "OK"); }
void handleXmas() { sendCommand("XMAS", "Xmas Mode"); server.send(200, "text/plain", "OK"); }
void handleOff() { sendCommand("OFF", "TURN OFF"); server.send(200, "text/plain", "OK"); }

void setup() {
  // 
  pinMode(BTN_UP, INPUT_PULLUP);
  pinMode(BTN_OK, INPUT_PULLUP);
  pinMode(BTN_DOWN, INPUT_PULLUP);

  // ESP32)
  Serial.begin(9600);

  // 
  Wire.begin(D2, D1);
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { for(;;); }
  
  // 
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println("Connecting WiFi...");
  display.display();

  // 
  WiFi.begin(ssid, password);
  int retry = 0;
  while (WiFi.status() != WL_CONNECTED && retry < 20) {
    delay(500);
    display.print(".");
    display.display();
    retry++;
  }

  //   IP
  display.clearDisplay();
  display.setCursor(0,0);
  if(WiFi.status() == WL_CONNECTED) {
    display.println("WiFi Connected!");
    display.println(WiFi.localIP());
  } else {
    display.println("WiFi Failed!");
    display.println("Manual Mode Only");
  }
  display.display();
  delay(2000);

  //  
  server.on("/", handleRoot);
  server.on("/fire", handleFire);
  server.on("/forest", handleForest);
  server.on("/xmas", handleXmas);
  server.on("/off", handleOff);
  server.begin();

  drawMenu();
}

void loop() {
  // 
  server.handleClient();

  
  if(digitalRead(BTN_UP) == LOW) {
    menuIndex--;
    if(menuIndex < 0) menuIndex = maxMenus - 1;
    drawMenu();
    delay(200);
  }
  
  if(digitalRead(BTN_DOWN) == LOW) {
    menuIndex++;
    if(menuIndex >= maxMenus) menuIndex = 0;
    drawMenu();
    delay(200);
  }


  if(digitalRead(BTN_OK) == LOW) {
    if(menuIndex == 0) sendCommand("FIRE", "Fire Mode");
    else if(menuIndex == 1) sendCommand("FOREST", "Forest Mode");
    else if(menuIndex == 2) sendCommand("XMAS", "Xmas Mode");
    else if(menuIndex == 3) sendCommand("OFF", "TURN OFF");
  }
}

//  
void drawMenu() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  
  //      
  display.setCursor(0, 0);
  if(WiFi.status() == WL_CONNECTED) display.println(WiFi.localIP());
  else display.println("Offline Mode");
  
  display.drawLine(0, 10, 128, 10, WHITE);

  for(int i=0; i<maxMenus; i++) {
    if(i == menuIndex) {
      display.fillRect(0, 15 + (i*12), 128, 10, WHITE);
      display.setTextColor(BLACK);
    } else {
      display.setTextColor(WHITE);
    }
    display.setCursor(5, 16 + (i*12));
    display.println(menuItems[i]);
  }
  display.display();
}
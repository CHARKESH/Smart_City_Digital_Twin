#include <WiFi.h>
#include <ThingSpeak.h>
#include <DHT.h>
#include "ESP32Servo.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

const char* ssid = "your_wifi_ssid";  // YOUR WiFi SSID
const char* password = "your_wifi_password";  // YOUR WiFi Password
unsigned long channelID = YOUR_CHANNEL_ID ;  // YOUR ThingSpeak Channel ID
const char* apiKey = "YOUR_WRITE_API_KEY";  // YOUR Write API Key

WiFiClient client;
WiFiServer server(80);

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define DHTPIN 15
#define DHTTYPE DHT22
Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
DHT dht(DHTPIN, DHTTYPE);
Servo gateServo;

// Pins
#define TRIG_PIN 5
#define ECHO_PIN 18
#define IR_PIN 4
#define LDR_PIN 27
#define SERVO_PIN 13
#define LED_PIN 25

String header, ledState = "auto";
String ldrMessage = "Ready";
String opticalMsg = "";
unsigned long currentTime, previousTime = 0;
const long timeoutTime = 2000;
bool ldrOverride = false;

// Morse Code - FIXED (correct character)
String morseCode[] = {".-", "-...", "-.-.", "-..", ".", "..-.", 
                      "--.", "-..", ".-", "..", ".---", "-.-", ".-..", 
                      "--", "-.", "---", ".--.", "--.-", ".-.", "...", 
                      "-", "..-", "...-", ".--", "-..-", "-.--", "--.."};
String letters[] = {"A", "B", "C", "D", "E", "F", "G", "H", "I", "J", 
                    "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", 
                    "U", "V", "W", "X", "Y", "Z"};

void oledPrintFullData(int parking, int lightLevel, float distance, float temp) {
  oled.clearDisplay();
  oled.setTextSize(1);
  oled.setTextColor(SSD1306_WHITE);
  oled.setCursor(0,0);
  
  oled.println("Park:" + String(parking ? "FULL" : "FREE"));
  oled.println("Ldr:" + String(lightLevel) + "%");
  oled.println("Dist:" + String(distance,0));
  oled.println("Web:" + ldrMessage.substring(0,8));
  oled.println("Opt:" + opticalMsg.substring(0,8));
  
  oled.display();
}

void checkOpticalMessage() {
  static unsigned long lastLdrChange = 0;
  static bool lastLdrState = false;
  static String currentMorse = "";
  
  bool ldrState = !digitalRead(LDR_PIN);
  
  if (ldrState != lastLdrState) {
    unsigned long duration = millis() - lastLdrChange;
    
    if (duration > 50 && duration < 300) {
      if (duration < 150) currentMorse += ".";
      else currentMorse += "-";
    } else if (duration > 300 && duration < 800) {
      if (currentMorse.length() > 0) {
        opticalMsg += decodeMorse(currentMorse);
        currentMorse = "";
      }
    } else if (duration > 1000) {
      if (currentMorse.length() > 0) {
        opticalMsg += decodeMorse(currentMorse);
        currentMorse = "";
        opticalMsg += " ";
      }
    }
    
    lastLdrState = ldrState;
    lastLdrChange = millis();
  }
}

String decodeMorse(String code) {
  for (int i = 0; i < 26; i++) {
    if (morseCode[i] == code) return letters[i];
  }
  return "?";
}

void setup() {
  Serial.begin(115200);
  
  pinMode(TRIG_PIN, OUTPUT); 
  pinMode(ECHO_PIN, INPUT);
  pinMode(IR_PIN, INPUT);
  pinMode(LDR_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  gateServo.attach(SERVO_PIN);
  
  dht.begin();
  oled.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  
  WiFi.begin(ssid, password);
  while(WiFi.status() != WL_CONNECTED) {
    delay(500); 
    Serial.print(".");
  }
  Serial.println("IP: " + WiFi.localIP().toString());
  
  server.begin();
  ThingSpeak.begin(client);
}

void loop() {
  checkOpticalMessage();
  
  WiFiClient webClient = server.accept();
  if (webClient) {
    currentTime = millis(); 
    previousTime = currentTime;
    String currentLine = "";
    while (webClient.connected() && currentTime - previousTime <= timeoutTime) {
      currentTime = millis();
      if (webClient.available()) {
        char c = webClient.read();
        header += c;
        if (c == '\n') {
         if (currentLine.length() == 0) {
           webClient.println("HTTP/1.1 200 OK");
           webClient.println("Content-type:text/html");
           webClient.println();
           
           int lightLevelRaw = digitalRead(LDR_PIN);
           int lightLevel = lightLevelRaw ? 20 : 80;
           
           if (header.indexOf("GET /msg?text=") >= 0) {
             int start = header.indexOf("text=") + 5;
             int end = header.indexOf(" ", start);
             if (end == -1) end = header.length();
             ldrMessage = header.substring(start, end).substring(0,15);
           }
           
           if (header.indexOf("GET /led/on") >= 0) {
             ledState = "on"; 
             ldrOverride = true; 
             digitalWrite(LED_PIN, HIGH);
           } else if (header.indexOf("GET /led/off") >= 0) {
             ledState = "off"; 
             ldrOverride = true; 
             digitalWrite(LED_PIN, LOW);
           } else if (header.indexOf("GET /led/auto") >= 0) {
             ledState = "auto"; 
             ldrOverride = false;
           }
           
           // CLEAN ENGLISH WEB PAGE
           webClient.println("<!DOCTYPE html><html>");
           webClient.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
           webClient.println("<style>body{background:#1a1a1a;color:#fff;font-family:Arial;text-align:center;padding:20px;font-size:16px;}");
           webClient.println(".btn{padding:12px 20px;font-size:16px;margin:5px;border:none;border-radius:8px;cursor:pointer;}");
           webClient.println(".on{background:#4CAF50;color:white;}.off{background:#f44336;color:white;}.auto{background:#ff9800;color:white;}.send{background:#2196F3;color:white;}");
           webClient.println("input{width:85%;padding:12px;font-size:16px;margin:10px;border-radius:5px;border:1px solid #444;}</style></head>");
           
           webClient.println("<body><h1>Smart City Control Panel v4.0</h1>");
           
           int parking = !digitalRead(IR_PIN);
           webClient.println("<h2>Parking Status: " + String(parking ? "FULL" : "FREE") + "</h2>");
           webClient.println("<h2>Street Light: " + ledState + "</h2>");
           webClient.println("Light Level: " + String(lightLevel) + "%<br><br>");
           
           webClient.println("<h3>Web Message Display</h3>");
           webClient.println("<b>" + ldrMessage + "</b><br>");
           webClient.println("<form action='/msg'>");
           webClient.println("<input name='text' placeholder='Enter web message' maxlength='15'>");
           webClient.println("<input type='submit' value='Send Message' class='btn send'>");
           webClient.println("</form>");
           
           webClient.println("<h3>Phone Flashlight Message</h3>");
           webClient.println("<b>" + opticalMsg + "</b><br>");
           webClient.println("<p><small>Flash Morse code: Short=Dot Long=Dash (SOS=...---...)</small></p>");
           
           webClient.println("<h3>Light Controls</h3>");
           webClient.println("<a href='/led/on'><button class='btn on'>Turn Light ON</button></a>");
           webClient.println("<a href='/led/off'><button class='btn off'>Turn Light OFF</button></a>");
           webClient.println("<a href='/led/auto'><button class='btn auto'>Auto Mode</button></a>");
           
           webClient.println("<hr><p>Control Panel IP Address: " + WiFi.localIP().toString() + "</p>");
           webClient.println("</body></html>");
           break;
         }
         currentLine = "";
       } else if (c != '\r') currentLine += c;
     }
   }
   header = ""; 
   webClient.stop();
 }
 
 // Sensors (5s) - FIXED
 static unsigned long lastUpdate = 0;
 if (millis() - lastUpdate > 5000) {
   int parking = !digitalRead(IR_PIN);
   int ldrRaw = digitalRead(LDR_PIN);
   int lightLevel = ldrRaw ? 20 : 80;
   
   if (!ldrOverride) digitalWrite(LED_PIN, ldrRaw);
   
   digitalWrite(TRIG_PIN, LOW); 
   delayMicroseconds(2);
   digitalWrite(TRIG_PIN, HIGH); 
   delayMicroseconds(10);
   digitalWrite(TRIG_PIN, LOW);
   float distance = pulseIn(ECHO_PIN, HIGH) * 0.034 / 2;
   float temp = dht.readTemperature();
   
   if (parking == 0 && distance < 15) gateServo.write(90);
   else gateServo.write(0);
   
   static unsigned long tsUpdate = 0;
   if (millis() - tsUpdate > 20000) {
     ThingSpeak.setField(1, distance);
     ThingSpeak.setField(2, parking);
     ThingSpeak.setField(3, temp);
     ThingSpeak.setField(4, lightLevel);
     ThingSpeak.setField(5, (float)opticalMsg.length());
     ThingSpeak.writeFields(channelID, apiKey);
     tsUpdate = millis();
   }
   
   oledPrintFullData(parking, lightLevel, distance, temp);
   
   Serial.println("Light=" + String(lightLevel) + "% WebMsg=[" + ldrMessage + 
                  "] PhoneFlash=[" + opticalMsg + "]");
   Serial.println("PARKING:" + String(parking ? "FULL" : "FREE") + 
                  " | LDR:" + String(lightLevel) + "%" + 
                  " | TEMP:" + String(temp,1) + "C" + 
                  " | DIST:" + String(distance,0) + "cm" +
                  " | GATE:" + String((distance < 15 && parking == 0) ? "OPEN" : "CLOSED"));
   
   lastUpdate = millis();
 }
}

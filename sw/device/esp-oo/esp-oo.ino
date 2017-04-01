//Wi-Fi
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

//Websockets
#include <WebSockets.h>
#include <WebSocketsServer.h>

//Webserver and page to be served included in PROGMEM within guihtml.h
#include <ESP8266WebServer.h>
#include "guihtml.h"

//mDNS
#include <ESP8266mDNS.h>

//EEPROM
#include <EEPROM.h>

//OTA
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

//Motor pinout definitions
#define M1A 14
#define M2A 5
#define M3A 15
#define M4A 2

#define M1B 13
#define M2B 4
#define M3B 12
#define M4B 0
char M_A_STR[9]={0,0,0,0,0,0,0,0,0};
char M_B_STR[9]={0,0,0,0,0,0,0,0,0};
uint32_t M_A = 0;
uint32_t M_B = 0;

//WiFi configuration
char ssid1[32] = "";
char pass1[32] = "";
char ssid2[32] = "";
char pass2[32] = "";
char hostn[32] = "";

ESP8266WiFiMulti WiFiMulti;
ESP8266WebServer webServer = ESP8266WebServer(80);
WebSocketsServer webSocket = WebSocketsServer(81);

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t payload_len) {
    switch(type) {
        case WStype_DISCONNECTED:
            Serial.printf("[%u] Disconnected!\n", num);
            break;
        case WStype_CONNECTED:
            {
                IPAddress ip = webSocket.remoteIP(num);
                Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
            }
            break;
        case WStype_TEXT:
            Serial.printf("[%u] get Text: %s\n", num, payload);

            if(payload[0] == '#') {
                strncpy ( M_A_STR, (char *) &payload[1], 8 );
                strncpy ( M_B_STR, (char *) &payload[9], 8 );
                M_A = (uint32_t) strtoul(M_A_STR, NULL, 16);
                M_B = (uint32_t) strtoul(M_B_STR, NULL, 16);
                analogWrite(M1A, ((M_A >> 24) & 0xFF));
                analogWrite(M2A, ((M_A >> 16) & 0xFF));
                analogWrite(M3A, ((M_A >> 8) & 0xFF));
                analogWrite(M4A, ((M_A >> 0) & 0xFF));
                analogWrite(M1B, ((M_B >> 24) & 0xFF));
                analogWrite(M2B, ((M_B >> 16) & 0xFF));
                analogWrite(M3B, ((M_B >> 8) & 0xFF));
                analogWrite(M4B, ((M_B >> 0) & 0xFF));
                Serial.printf("A - %u,%u,%u,%u\n", ((M_A >> 24) & 0xFF),((M_A >> 16) & 0xFF),((M_A >> 8) & 0xFF),((M_A >> 0) & 0xFF) );
                Serial.printf("B - %u,%u,%u,%u\n", ((M_B >> 24) & 0xFF),((M_B >> 16) & 0xFF),((M_B >> 8) & 0xFF),((M_B >> 0) & 0xFF) );
                webSocket.sendTXT(num, "_OO_");
            }
            break;
        case WStype_BIN:
            Serial.printf("[%u] get binary length: %u\n", num, payload_len);
            //hexdump(payload, payload_len);
            break;
    }
}


void setup() {
  int retries = 0;
  
  Serial.begin(115200);
  delay(10);
  //Serial.setDebugOutput(true);

  //Load WiFi
  Serial.println("Loading Wi-Fi config");
  EEPROM.begin(512);
  EEPROM.get(0, ssid1);
  EEPROM.get(0+sizeof(ssid1), pass1);
  EEPROM.get(0+sizeof(ssid1)+sizeof(pass1), ssid2);
  EEPROM.get(0+sizeof(ssid1)+sizeof(pass1)+sizeof(ssid2), pass2);
  EEPROM.get(0+sizeof(ssid1)+sizeof(pass1)+sizeof(ssid2)+sizeof(pass2), hostn);

  Serial.println(ssid1);
  Serial.println(pass1);
  Serial.println(ssid2);
  Serial.println(pass2);
  Serial.println(hostn);
  WiFi.hostname(hostn);
  WiFi.mode(WIFI_AP_STA);
  WiFiMulti.addAP(ssid1, pass1);
  WiFiMulti.addAP(ssid2, pass2);

  while(retries < 20){
    if(WiFiMulti.run() == WL_CONNECTED) { 
      break;
    }
    delay(500);
    Serial.println("Attempting Wi-Fi connection");
    retries++;
  }
  
  if(WiFiMulti.run() == WL_CONNECTED){
    WiFi.mode(WIFI_STA);
    Serial.println("");
    Serial.println("Wi-Fi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    ArduinoOTA.setHostname(hostn);
  } else {
    WiFi.softAP("ESPOO-AP");
    Serial.println("");
    Serial.println("WiFi AP Started");
    Serial.println("IP address: ");
    Serial.println(WiFi.softAPIP());    
  }
  
  ArduinoOTA.onStart([]() {
    Serial.println("\nStart");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
    ESP.restart();
  });

  ArduinoOTA.begin();
  Serial.println("OTA service started");
  
  analogWriteRange(255);
  pinMode(M1A, OUTPUT);
  pinMode(M2A, OUTPUT);
  pinMode(M3A, OUTPUT);
  pinMode(M4A, OUTPUT);
  pinMode(M1B, OUTPUT);
  pinMode(M2B, OUTPUT);
  pinMode(M3B, OUTPUT);
  pinMode(M4B, OUTPUT);

  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  webServer.on("/", []() {
    webServer.send(200, "text/html", GUIHTML);
  });
  webServer.on("/config.js", []() {
    String output = "var ssid1='" + String(ssid1) + "';";
    output += "var pass1='" + String(pass1) + "';";
    output += "var ssid2='" + String(ssid2) + "';";
    output += "var pass2='" + String(pass2) + "';";
    output += "var hostname='" + String(hostn) + "';";
    webServer.send(200, "text/json", output);
  });
  webServer.on("/configwifi", []() {
    Serial.println("Saving WiFi Config");
    webServer.arg("ssid1").toCharArray(ssid1, sizeof(ssid1)-1);
    webServer.arg("pass1").toCharArray(pass1, sizeof(pass1)-1);
    webServer.arg("ssid2").toCharArray(ssid2, sizeof(ssid2)-1);
    webServer.arg("pass2").toCharArray(pass2, sizeof(pass2)-1);
    webServer.arg("hostname").toCharArray(hostn, sizeof(hostn)-1);
    
    EEPROM.put(0, ssid1);
    EEPROM.put(0+sizeof(ssid1), pass1);
    EEPROM.put(0+sizeof(ssid1)+sizeof(pass1), ssid2);
    EEPROM.put(0+sizeof(ssid1)+sizeof(pass1)+sizeof(ssid2), pass2);
    EEPROM.put(0+sizeof(ssid1)+sizeof(pass1)+sizeof(ssid2)+sizeof(pass2), hostn);
    EEPROM.commit();
    EEPROM.end();
    webServer.send(200, "text/html", "Config saved - Rebooting, please reconnect.");
    Serial.println("Rebooting...");
    ESP.reset();
  });
  webServer.begin();
    
  if(MDNS.begin(hostn)) {
    Serial.println("MDNS responder started");
  }
  MDNS.addService("http", "tcp", 80);
  MDNS.addService("ws", "tcp", 81);
}

void loop() {
  webSocket.loop();
  webServer.handleClient();
  ArduinoOTA.handle();
}

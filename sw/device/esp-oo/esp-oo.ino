//Wi-Fi
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

//Websockets
#include <WebSockets.h>
#include <WebSocketsServer.h>

//Webserver and page to be served included in PROGMEM within webpage.h
#include <ESP8266WebServer.h>
#include "webpage.h"

//mDNS
#include <ESP8266mDNS.h>

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
                strncpy ( M_A_STR, (const char *) &payload[1], 8 );
                strncpy ( M_B_STR, (const char *) &payload[9], 8 );
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
            }
            
            break;
        case WStype_BIN:
            Serial.printf("[%u] get binary length: %u\n", num, payload_len);
            hexdump(payload, payload_len);

            break;
    }
}

void setup() {
  Serial.begin(115200);
  delay(10);

  Serial.setDebugOutput(true);

  Serial.println();
  
  WiFiMulti.addAP("www.johnchan.net", "WhereIsHuey");
  WiFiMulti.addAP("www.madox.net", "WhereIsCarKey");
  while(WiFiMulti.run() != WL_CONNECTED) {
    delay(100);
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

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
    webServer.send(200, "text/html", WEBPAGE);
  });
  webServer.begin();
    
  if(MDNS.begin("esp-oo")) {
    Serial.println("MDNS responder started");
  }
  MDNS.addService("http", "tcp", 80);
  MDNS.addService("ws", "tcp", 81);
}

void loop() {
  webSocket.loop();
  webServer.handleClient();
}

#include <Arduino.h>
#include <SSD1306.h>
#include <DHTesp.h>
#include <stdio.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ConfigPortal8266.h>


SSD1306             display(0x3c, 4, 5, GEOMETRY_128_32);
DHTesp              dht;
int                 interval = 2000;
unsigned long       lastDHTReadMillis = 0;
float               humidity = 0;
float               temperature = 0;
char what[15];

char*               ssid_pfix = (char*)"CaptivePortal";
String              user_config_html = "";   
const char* ssi = "";
const char* passwor = "";

ESP8266WebServer server(80);

void handleRoot(){
    String message = (server.method()== HTTP_GET)?"GET":"POST";
    message += " " + server.uri() + "\n";
    for (uint8_t i=0; i<server.args();i++){
        message += " " + server.argName(i) + " : " + server.arg(i) + "\n";
    }
    message += "\n Hello form ESP8266 test\n\n";
    message += "Temp / Humid\n";
    message += what;
    message += "\n";
    server.send(200,"text/plain",message);
}

void handleNotFound(){
    String message = "File not found\n\n";
    server.send(404, "text/plain", message);
}


void setup() {
  // put your setup code here, to run once:
  
  Serial.begin(115200);
  dht.setup(14, DHTesp::DHT22); // Connect DHT sensor to GPIO 14
  delay(1000);
  //Serial.println(); Serial.println("Humidity (%)\tTemperature (C)");

loadConfig();
    // *** If no "config" is found or "config" is not "done", run configDevice ***
    if(!cfg.containsKey("config") || strcmp((const char*)cfg["config"], "done")) {
        configDevice();
    }
    WiFi.mode(WIFI_STA);
    WiFi.begin((const char*)cfg["ssid"], (const char*)cfg["w_pw"]);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    // main setup
    Serial.printf("\nIP address : "); Serial.println(WiFi.localIP());
if (MDNS.begin("miniwifi")) {
        Serial.println("MDNS responder started");
    } 

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssi,passwor);

  while(WiFi.status() !=WL_CONNECTED){
      delay(250);
      Serial.print(".");
  }
    Serial.print("portal set to ");
    Serial.println(ssi);
    Serial.print("IP address : ");
    Serial.println(WiFi.localIP());

    if (MDNS.begin("miniWeb")){
        Serial.println("MDNS responded");

    }
    server.on("/", handleRoot);
    server.on("/inline",[](){
        server.send(200,"text/plain","hello from inline func\n");
    });
    server.onNotFound(handleNotFound);
    server.begin();
    Serial.println("HTTP Server STarted");
     

}

void readDHT22() {
    unsigned long currentMillis = millis();

    if(currentMillis - lastDHTReadMillis >= interval) {
        lastDHTReadMillis = currentMillis;

        humidity = dht.getHumidity();              // Read humidity (percent)
        temperature = dht.getTemperature();        // Read temperature as Fahrenheit
    }
    delay(1000);
}

void loop() {
  // put your main code here, to run repeatedly
    readDHT22();
    MDNS.update();
    server.handleClient();

    display.init();
    display.flipScreenVertically();
    display.setFont(ArialMT_Plain_16);
    sprintf(what,"%.2f, %.2f",temperature,humidity);
    
    display.drawString(10, 10, what);
    
    Serial.printf("%.1f\t %.1f\n", temperature, humidity);
    display.display();
       
}
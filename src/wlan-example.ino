#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h> 

ESP8266WebServer server(80);    // Server Port  hier einstellen

struct wlanconfig
{
 String ssid;
 String password;
};

typedef struct wlanconfig WlanConfig;

int addr_ssid = 0;
int addr_password = 30; 
int max_ssid_length = addr_password;
int max_password_length = 100;

String formdata = "<html>  <body>    <form method=\"get\" action=\"/save\">        <label for=\"ssid\">SSID:</label><br> <input type=\"text\" id=\"ssid\" name=\"ssid\"><br>  <label for=\"password\">Password:</label><br>  <input type=\"text\" id=\"password\" name=\"password\">  <input type=\"submit\" value=\"Submit\">    </form>  </body></html>";



void setup() {
    Serial.begin(115200);         // Serielle schnittstelle initialisieren
    Serial.println("");           // Lehere Zeile ausgeben
    if(!configureWlan()){
      configureAP();
    }

   server.on("/", form);
   server.on("/save", save);
   server.on("/restart", restart);
  server.begin();               // Starte den Server
  Serial.println("HTTP Server gestartet");
}

void loop() {
  // put your main code here, to run repeatedly:
  server.handleClient();
}


void configureAP(){
   Serial.println("Starte WLAN-Hotspot \"astral\"");
  WiFi.mode(WIFI_AP);           // access point modus
  WiFi.softAP("astral", "12345678");    // Name des Wi-Fi netzes
  delay(500);                   //Abwarten 0,5s
  Serial.print("IP Adresse ");  //Ausgabe aktueller IP des Servers
  Serial.println(WiFi.softAPIP()); 
}

boolean configureWlan(){
  WlanConfig config = readWlanConfig();
  if(config.ssid != NULL && config.ssid.length() > 0 
      && config.password != NULL && config.password.length() > 0){
      WiFi.begin(config.ssid, config.password);             // Connect to the network
      Serial.print("Connecting to ");
      Serial.print(config.ssid); Serial.println(" ...");
      int i = 0;
      while (WiFi.status() != WL_CONNECTED && i < 60) { // Wait for the Wi-Fi to connect
        delay(1000);
        Serial.print(++i); Serial.print(' ');
      }
      if(WiFi.status() == WL_CONNECTED){
        Serial.print("Wlan Connected");
        return true;
      }
      Serial.print("Wlan Timeout");
  }

  return false;
  
}


void form(){
  server.send(200, "text/html", formdata);
}

void save(){
  WlanConfig config;
  if(server.hasArg("ssid") && server.hasArg("password"))
  {
    config.ssid = server.arg("ssid");
    config.password = server.arg("password");
    writeWlanConfig(config);
    server.send(200, "text/html", "<html><body>Daten gespeichert. <a href=\"/restart\"> Restart </a></body></html>");
  }else{
    server.send(200, "text/html", "<html><body>Fehler</body></html>");
  }
}

void restart(){
  ESP.restart();
}



WlanConfig readWlanConfig(){
  WlanConfig wlanconfig;
  EEPROM.begin(512);

  for (int l = addr_ssid; l < max_ssid_length; ++l) {
    byte data = EEPROM.read(l);
    if(data == 0){
      break;
    }
    wlanconfig.ssid += char(data); 
  }
  

  for (int l = addr_password; l < max_password_length; ++l) {
    byte data = EEPROM.read(l);
    if(data == 0){
      break;
    }
    wlanconfig.password += char(data); 
  }
  Serial.print("SSID:"+wlanconfig.ssid);
  Serial.print("PASSWORD:"+wlanconfig.password);
  EEPROM.commit();
  return wlanconfig;
}

void writeWlanConfig(WlanConfig wlanconfig){
  if(wlanconfig.ssid.length() > max_ssid_length){
    Serial.print("Error: SSID zu lang");
    return; 
  }
  if(wlanconfig.password.length() > max_password_length){
    Serial.print("Error: SSID zu lang");
    return; 
  }
  EEPROM.begin(512);
  writeEPROM(addr_ssid,wlanconfig.ssid,max_ssid_length);
  writeEPROM(addr_password,wlanconfig.password,max_password_length);
  EEPROM.commit();
}

void writeEPROM(int start,String value,int max){
  for(int x = 0; x < max; x++){
    if(x < value.length()){
       EEPROM.write(start+x, value[x]);
    }else{
      EEPROM.write(start+x, 0);
    }
  }

}

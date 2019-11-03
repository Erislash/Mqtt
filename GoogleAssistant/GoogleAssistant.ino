#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

#define led  2
#define ssid  "RCA"
#define wifiPass "12345678"
#define mqttServer "mqtt.beebotte.com"
#define mqttPort 1883
#define mqttUser "token_YcwMRaLbK8Us4SCb"
#define subTopic "Ifttt/ghome"


WiFiClient espClient;
PubSubClient client(espClient);

void setup(){
  Serial.begin(115200);
  pinMode(led, OUTPUT);

  ConnectWifi(ssid, wifiPass);

  client.setServer(mqttServer, mqttPort);

  Serial.println("Conectando a MQTT");

  
  while(!client.connected()){
    Serial.println(".");
    
    if(client.connect("NodeLuz", mqttUser, "")){
      Serial.println("Conectado");
      
      client.subscribe(subTopic);
      Serial.println("Suscripto al Tema: ");
      Serial.println(subTopic);
      client.setCallback(CallMQTT);
    }else{
      Serial.print("No se pudo Conectar al Broker MQTT");
      Serial.print(client.state());
      delay(2000);
    }
  }
 client.setCallback(CallMQTT);
}


void loop(){
  client.loop();
}


void ConnectWifi(String p_ssid, String p_password){
  WiFi.mode(WIFI_OFF);
  delay(1000);
  WiFi.mode(WIFI_STA);
  WiFi.begin(p_ssid, p_password);

  Serial.println();
  Serial.print("Intentando Conectar a la Red: ");
  Serial.println(p_ssid);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Conectado a la Red");
}


void CallMQTT(char* topic, byte* payload, unsigned int length) {
  String message = "";
 
  Serial.print("Ha llegado un mensaje desde el tema: ");
  Serial.println(topic);
 
  Serial.print("Mensaje:");
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.println(message);
  
  StaticJsonDocument<256> doc;


  //char* action = new char;
  //message.toCharArray(action, message.length() + 1);
   
  DeserializationError error = deserializeJson(doc, message);

  if(error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    return;
  }

  boolean action = doc["data"];
  Serial.println(action);
  //delete[] action;
  Serial.println();
  Serial.println("/////////////////////////////////////////////////////////////////");


  if(action)
    digitalWrite(led, LOW);
  else if(!action)
    digitalWrite(led, HIGH);
}

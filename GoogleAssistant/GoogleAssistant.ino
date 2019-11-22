//LIBRERIAS A USAR
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

//CONFIGURACIÓNES INICIALES
#define led  2
#define ssid  "RED"
#define wifiPass "12345678"
#define mqttServer "mqtt.beebotte.com"
#define mqttPort 1883
#define mqttUser "token_YcwMxxxxxxxxxxxxxx"   //TOKEN QUE NOS PROVEE BEEBOTTE
#define subTopic "Ifttt/ghome"                //DIRECCIÓN QUE ASIGNAMOS ANTERIORMENTE

WiFiClient espClient;
PubSubClient client(espClient);

long lastAttempt = 0;

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
//RECONEXIÓN AL SERVIDOR
  if (!client.connected()) {
    long now = millis();
    if (now - lastAttempt > 5000) {
      lastReconnectAttempt = now;
      if (reconnect()) {
        lastAttempt = 0;
      }
    }
  } else {
    client.loop();
  }
}


//FUNCIÓN PARA CONECTARSE A UNA RED WIFI
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


/*CALLBACK, SE EJECUTARÁ AL RECIBIR UN MENSAJE DEL TEMA 
AL QUE NOS SUSCRIBIMOS*/
void CallMQTT(char* topic, byte* payload, unsigned int length) {
  String message = "";
 
  Serial.print("Ha llegado un mensaje desde el tema: ");
  Serial.println(topic);
 
  Serial.print("Mensaje:");
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.println(message);

/*DESERIALIZACIÓN DEL JSON QUE NOS LLEGA A TRAVÉS DEL BROKER, 
ENVIADO POR EL WEBHOOK*/
  StaticJsonDocument<256> doc;

  DeserializationError error = deserializeJson(doc, message);

  if(error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    return;
  }

//EXTRAEMOS EL DATO GUARDADO EN LA KEY "data"
  boolean action = doc["data"];
  Serial.println();
  Serial.println("/////////////////////////////////////////////////////////////////");

/*SI EL MENSAJE QUE RECIBIMOS ES "true" ENCENDEMOS EL LED
E INFORMAMOS QUE SE ENCENDIO A TRAVÉS DE OTRA DIRECCIÓN
A LA QUE SE SUSCRIBIRÁ LA APLICACIÓN IoT "MQTT Panel"*/ 
  if(action){
    digitalWrite(led, LOW);
    client.publish("Ifttt/nodeStatus", "{\"data\":true}");
  }
//DE RECIBIR UN "false", SE APAGARÁ E INFORMARÁ  
  else if(!action){
    digitalWrite(led, HIGH);
    client.publish("Ifttt/nodeStatus", "{\"data\":false}");
  }
}

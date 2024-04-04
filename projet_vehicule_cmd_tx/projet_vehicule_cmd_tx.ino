


#include <WiFi.h>

// Pins used for the joystick
#define RX   32 // Control the movements according to RX
#define RY   33 // Control the movements according to RY
// Pins used by the LED to indicate the direction of the car
#define LED_R 13  // Indicates right turn
#define LED_L 14  // Indicates left turn
#define LED_F 22  // Indicates forward motion 
#define LED_B 23  // Indicates reverse motion

// Constantes
#define DEFAULT     50
#define IT          20

#define FORWARD     1
#define BACKWARD    2
#define LEFT        3 
#define RIGHT       4 
#define STOP        5
#define AUTO_MODE   8
#define MANUEL_MODE 9


const char* ssid     = "WIFI_CAR"; // WiFi network name
const char* password = "MON_PROJET_2024";  // WiFi network password
// Replace with the MAC address of the WiFi access point
uint8_t mac[] = {0xA0, 0xB7, 0x65, 0x56, 0xD0, 0xE5}; 
const int serverPort = 80;

WiFiClient client;

void setup() {
  Serial.begin(115200);

  // Pin configuration
  pinMode(RX,INPUT); 
  pinMode(RY,INPUT); 
  pinMode(LED_R, OUTPUT);
  pinMode(LED_L, OUTPUT);
  pinMode(LED_F, OUTPUT);
  pinMode(LED_B, OUTPUT);

  // Connecting to the WiFi network with the specified MAC address
  Serial.println("Connecting to the WiFi network...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid,password); 
  WiFi.macAddress(mac); // Use the specified MAC address

  long init_t = millis();
  bool etat = false;
  
  while (WiFi.status() != WL_CONNECTED) {
    if ((millis()-init_t) >= 500){
      Serial.print(".");
      etat = not etat;
      digitalWrite(LED_F, etat);
      init_t = millis();
    }
  }

  Serial.println("Connected");
  digitalWrite(LED_F, LOW);
}

void loop() {

  if (WiFi.status() == WL_CONNECTED) {

    if (client.connect(mac, serverPort)){
      sendData();
      client.stop();
      Serial.println("Données envoyées avec succès");
    } 
    else {
      Serial.println("Impossible de se connecter au serveur");
    }

    delay(100);
  } 
  else {
    Serial.println("Connexion WiFi perdue, réessayer...");
    delay(3000);
    ESP.restart();
  }
}


void sendData(void){
  int rx_value = map(analogRead(RX),0,4096,0,100); // Avance
  int ry_value = map(analogRead(RY),0,4096,0,100); // Virage

  // STOP
  if((rx_value >=(DEFAULT-IT) and rx_value <=(DEFAULT+IT)) and (ry_value >=(DEFAULT-IT) and ry_value <=(DEFAULT+IT))){
    client.println(STOP);
    digitalWrite(LED_F,LOW);
    digitalWrite(LED_B,LOW);
    digitalWrite(LED_L,LOW);
    digitalWrite(LED_R,LOW);
    Serial.println(STOP);
  }
  // FORWARD
  else if(rx_value >(DEFAULT+IT)){
    client.println(FORWARD);
    digitalWrite(LED_F,HIGH);
    Serial.println(FORWARD);
  }
  // BACKWARD
  else if(rx_value < (DEFAULT-IT)){
    client.println(BACKWARD);
    digitalWrite(LED_B,HIGH);
    Serial.println(BACKWARD);
  }
  // LEFT
  else if(ry_value <(DEFAULT-IT)){
    client.println(LEFT);
    digitalWrite(LED_L,HIGH);
    Serial.println(LEFT);
  }
  // RIGHT
  else if(ry_value > (DEFAULT+IT)){
    client.println(RIGHT);
    digitalWrite(LED_R,HIGH);
    Serial.println(RIGHT);
  }
}

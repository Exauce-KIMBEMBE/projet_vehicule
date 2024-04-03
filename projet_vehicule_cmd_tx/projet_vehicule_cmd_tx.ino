


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
#define DEFAULT_RX 525
#define DEFAULT_RY 525


const char* ssid     = "WIFI_CAR"; // WiFi network name
const char* password = "MON_PROJET_2024";  // WiFi network password
// Replace with the MAC address of the WiFi access point
uint8_t mac[] = {0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC}; 
const int serverPort = 80;

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
    
    WiFiClient client;

    if (client.connect(mac, serverPort)) {
      for (int i = 0; i < 10; i++) {
        client.println(i);
        delay(1000);
      }
      client.stop();
      Serial.println("Données envoyées avec succès");
    } 
    else {
      Serial.println("Impossible de se connecter au serveur");
    }
  } 
  else {
    Serial.println("Connexion WiFi perdue, réessayer...");
    delay(3000);
    ESP.restart();
  }

}
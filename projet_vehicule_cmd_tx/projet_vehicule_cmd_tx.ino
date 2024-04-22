/**
 * e-mail  : openprogramming23@gmail.com
 * Date    : 06/04/2024
 * Auteurs : @Exaucé KIMBEMBE / OpenProgramming
 *        
 *  
 * @Board : ESP32
 * 
 * This program handles the operation of a remote control designed to manage a vehicle via a Wi-Fi network. 
 * The code connects to a server established by an ESP32 board mounted on the vehicle and establishes 
 * a Wi-Fi connection in AP mode. 
 *  @@ The necessary materials to run the program include:
 *    ** An ESP32 board
 *    ** A joystick
 *    ** Two push buttons
 *    ** An I2C LCD screen
 *    ** 5 LEDs
 * 
 */

#ifndef ESP32
	#error "This code has been tested on an ESP32 board."
#endif

#include <WiFi.h>
#include <LiquidCrystal_I2C.h>

// Pins used for the joystick
#define RX   32 // Control the movements according to RX (FORWARD AND BACKWARD)
#define RY   33 // Control the movements according to RY
// Pins used by the LED to indicate the direction of the car
#define LED_R 13  // Indicates right turn
#define LED_L 14  // Indicates left turn
#define LED_F 18  // Indicates forward motion 
#define LED_B 23  // Indicates reverse motion
#define LED_S 15  // Indicates the state of wifi
// Pins used by the buttons
#define BTN_MANUEL   19 // Manual control button
#define BTN_AUTO     12 // Line follower control button

// Constantes
#define DEFAULT     50
#define IT          30
// Values to send
#define FORWARD     1
#define BACKWARD    2
#define LEFT        3 
#define RIGHT       4 
#define STOP        5
#define AUTO_MODE   8
#define MANUEL_MODE 9

// Messages
#define MSG_MODE_1  "  Control mode  "
#define MSG_MODE_2  " Line follower  "
#define MSG_S       "      STOP      "
#define MSG_F       "    FORWARD     "
#define MSG_B       "    BACKWARD    "
#define MSG_L       "   TURN LEFT    "
#define MSG_R       "   TURN RIGHT   "

const char* ssid     = "WIFI_CAR";        // WiFi network name
const char* password = "MON_PROJET_2024"; // WiFi network password
const int serverPort = 80;

WiFiClient client;
LiquidCrystal_I2C lcd(0x3E,16,2);
IPAddress serverIP(192, 168, 4, 1);

// Function prototypes
void ledOff(void);
void sendData(void);


void setup() {
  Serial.begin(115200);
  // lcd configuration
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(MSG_MODE_1);
  lcd.setCursor(0,1);
  lcd.print(MSG_S);

  // Pin configuration
  pinMode(RX,INPUT); 
  pinMode(RY,INPUT); 
  pinMode(LED_R, OUTPUT);
  pinMode(LED_L, OUTPUT);
  pinMode(LED_F, OUTPUT);
  pinMode(LED_B, OUTPUT);
  pinMode(LED_S, OUTPUT);
  pinMode(BTN_AUTO, INPUT_PULLUP);
  pinMode(BTN_MANUEL, INPUT_PULLUP);
  
  digitalWrite(LED_S, LOW);
  
  // Connecting to the WiFi network with the specified MAC address
  Serial.println("Connecting to the WiFi network...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid,password); 
  
  while (WiFi.status() != WL_CONNECTED) {
      Serial.print(".");
      delay(500);
    }

  Serial.println("Connected");
  digitalWrite(LED_S, HIGH);
}


void loop() {

  if (WiFi.status() == WL_CONNECTED) {

    if (client.connect(serverIP, serverPort)){
      // controlMode
      if (not digitalRead(BTN_MANUEL)==true){
        
        client.println(MANUEL_MODE);
        client.stop();
        Serial.println(MANUEL_MODE);

        lcd.setCursor(0,0);
        lcd.print(MSG_MODE_1);
        
        delay(100);
        return;
      }
      // lineFollowerMode
      else if (not digitalRead(BTN_AUTO)==true){
        
        client.println(AUTO_MODE);
        client.stop();
        Serial.println(MANUEL_MODE);
        
        lcd.setCursor(0,0);
        lcd.print(MSG_MODE_2);
        lcd.setCursor(0,1);
        lcd.print("                ");

        delay(100);
        return;
      }
      else{
        lcd.setCursor(0,1);
        sendData();
        client.stop();
      }
    } 
    else {
      Serial.println("Impossible de se connecter au serveur");
    }

    delay(10);
  } 
  else {
    Serial.println("Connexion WiFi perdue, réessayer...");
    digitalWrite(LED_S, LOW);
    delay(3000);
    ESP.restart();
  }
}


void sendData(void){
  int rx_value = map(analogRead(RX),0,4095,0,100); // Avance
  int ry_value = map(analogRead(RY),0,4095,0,100); // Virage

  // STOP
  if((rx_value >=(DEFAULT-IT) and rx_value <= (DEFAULT+IT)) and (ry_value >= (DEFAULT-IT) and ry_value <= (DEFAULT+IT))){
    client.println(STOP);
    lcd.print(MSG_S);
    ledOff();
    Serial.println(STOP);
  }
  // FORWARD
  else if(rx_value > (DEFAULT+IT)){
    client.println(FORWARD);
    lcd.print(MSG_F);
    ledOff();
    digitalWrite(LED_F,HIGH);
    Serial.println(FORWARD);
  }
  // BACKWARD
  else if(rx_value < (DEFAULT-IT)){
    client.println(BACKWARD);
    lcd.print(MSG_B);
    ledOff();
    digitalWrite(LED_B,HIGH);
    Serial.println(BACKWARD);
  }
  // LEFT
  else if(ry_value < (DEFAULT-IT)){
    client.println(LEFT);
    lcd.print(MSG_L);
    ledOff();
    digitalWrite(LED_L,HIGH);
    Serial.println(LEFT);
  }
  // RIGHT
  else if(ry_value > (DEFAULT+IT)){
    client.println(RIGHT);
    lcd.print(MSG_R);
    ledOff();
    digitalWrite(LED_R,HIGH);
    Serial.println(RIGHT);
  }
}


void ledOff(void){
  digitalWrite(LED_F,LOW);
  digitalWrite(LED_B,LOW);
  digitalWrite(LED_L,LOW);
  digitalWrite(LED_R,LOW);
  delay(2);
}

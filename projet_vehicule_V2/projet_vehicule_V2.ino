/**
 * e-mail  : openprogramming23@gmail.com
 * Date    : 08/03/2024
 * Auteurs : @Exaucé KIMBEMBE / OpenProgramming
 *           @Write your name
 *  
 * @Board : Arduino, ESP32
 * 
 * This program has been implemented on the Arduino and ESP32 boards. 
 * The program is designed to create a vehicle equipped with an automatic 
 * mode (line follower) and a manual mode (controlled via Bluetooth).
 * @@@ The Wi-Fi option is only available on the ESP32 board.
 */

// Set the WiFi constant to false to use Bluetooth and true for WiFi.
#define WIFI true 

#ifdef __AVR__ 
      #define WIFI false
      #include <SoftwareSerial.h>

      SoftwareSerial bluetooth(2, 3); // RX, TX

      #define enA 5  // Enable1 L298 Pin enA 
      #define in1 8  // Motor1  L298 Pin in1 
      #define in2 9  // Motor1  L298 Pin in1 
      #define in3 10 // Motor2  L298 Pin in1 
      #define in4 11 // Motor2  L298 Pin in1 
      #define enB 6  // Enable2 L298 Pin enB 
      #define R_S A0 // Capteur IR droit
      #define L_S A1 // Capteur IR gauche 

#elif ESP32
      #if WIFI == false
            #include <BluetoothSerial.h>

            #if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
                  #error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
            #endif

            BluetoothSerial bluetooth;

            // Function prototypes
            void bt_receive(void);
            void select_mode(void);

      #else
            #include <WiFi.h>

            // WiFi Access Point created
            const char* ssid_ap     = "WIFI_CAR"; // WiFi network name
            const char* password_ap = "MON_PROJET_2024";  // WiFi network password

            IPAddress local_IP(192, 168, 4, 1); // Static IP address for the WiFi AP
            IPAddress gateway(192, 168, 4, 1);  // Default gateway IP address
            IPAddress subnet(255, 255, 255, 0); // Subnet mask

            WiFiServer server(80);

            // Function prototypes
            void WifiConnection(void);
            void WiFireceive(void);
      #endif

      #define enA 15 // Enable1 L298 Pin enA 
      #define in1 12 // Motor1  L298 Pin in1 
      #define in2 16 // Motor1  L298 Pin in1 
      #define in3 17 // Motor2  L298 Pin in1 
      #define in4 5  // Motor2  L298 Pin in1 
      #define enB 18 // Enable2 L298 Pin enB
      #define R_S 19 // Capteur IR droit
      #define L_S 21 // Capteur IR gauche 

      // PWM Parameter
      #define FREQ        1000 // Channel frequency in Hz
      #define CHANNEL     0    // Channel number used
      #define RESOLUTION  8    // 8 bits (0 - 255)
#endif

int Speed = 100; // The motor speed ranges from 0 to 255.  
int recv_data;   // variable to receive data 
int mode  = 0;

// Function prototypes
void forword(void);
void backword(void);
void turnRight(void);
void turnLeft(void);
void Stop(void);

void setup(){
      Serial.begin(115200);
      Serial.println("using serial monitor");
      // Pin configuration
      pinMode(R_S, INPUT);  // Declare ir sensor as input  
      pinMode(L_S, INPUT);  // Declare ir sensor as input
      pinMode(enA, OUTPUT); // Declare as output for L298 Pin enA 
      pinMode(in1, OUTPUT); // Declare as output for L298 Pin in1 
      pinMode(in2, OUTPUT); // Declare as output for L298 Pin in2 
      pinMode(in3, OUTPUT); // Declare as output for L298 Pin in3   
      pinMode(in4, OUTPUT); // Declare as output for L298 Pin in4 
      pinMode(enB, OUTPUT); // Declare as output for L298 Pin enB 

      #ifdef ESP32
            #if WIFI == false
                  bluetooth.begin("MY_CAR"); // Bluetooth name
            #else
                  WifiConnection();   // WiFi connection
            #endif

            // PWM Channel Initialization
            // It's possible to connect multiple pins to the same PWM channel on the ESP32.
            ledcSetup(CHANNEL, FREQ, RESOLUTION); // Configure the PWM channel
            ledcWrite(CHANNEL, Speed);   // Set the output signal of the channel
            ledcAttachPin(enA, CHANNEL); // Attach the pin to the PWM channel
            ledcAttachPin(enB, CHANNEL); // Attach the pin to the PWM channel
      #elif __AVR__
            bluetooth.begin(9600);
      #endif
}


void loop(){  
      #if WIFI == false
            #ifdef ESP32
                  if(bluetooth.connected()==true){
                        bt_receive();
                  }
                        
                  ledcWrite(CHANNEL, Speed);// Set the output signal of the channel
            #elif __AVR__
                  bt_receive();
                  analogWrite(enA, Speed); // Write The Duty Cycle 0 to 255 Enable Pin A for Motor1 Speed 
                  analogWrite(enB, Speed); // Write The Duty Cycle 0 to 255 Enable Pin B for Motor2 Speed 
            #endif 
      #else
            WiFireceive();
            ledcWrite(CHANNEL, Speed);// Set the output signal of the channel
      #endif

      if(mode==0){     
            //===============================================================================
            //                          Key Control Command
            //=============================================================================== 
            if(recv_data == 1){forword(); }  // if the recv_data is '1' the DC motor will go forward/Avant
            else if(recv_data == 2){backword();}  // if the recv_data is '2' the motor will Reverse/Aeeière
            else if(recv_data == 3){turnLeft();}  // if the recv_data is '3' the motor will turn left/Gauche
            else if(recv_data == 4){turnRight();} // if the recv_data is '4' the motor will turn right/Droite
            else if(recv_data == 5){Stop(); }     // if the recv_data '5' the motor will Stop/Stop

            //===============================================================================
            //                          Voice Control Command
            //===============================================================================    
            else if(recv_data == 6){turnLeft();  delay(400);  recv_data = 5;}
            else if(recv_data == 7){turnRight(); delay(400);  recv_data = 5;}
      }
      else{    
            //===============================================================================
            //                          Line Follower Control/Suivi de ligne
            //===============================================================================   
            //if Right Sensor and Left Sensor are at White color then it will call forword function  
            if((digitalRead(R_S) == 0)&&(digitalRead(L_S) == 0)){forword();} 
            //if Right Sensor is Black and Left Sensor is White then it will call turn Right function  
            if((digitalRead(R_S) == 1)&&(digitalRead(L_S) == 0)){turnRight();} 
            //if Right Sensor is White and Left Sensor is Black then it will call turn Left function
            if((digitalRead(R_S) == 0)&&(digitalRead(L_S) == 1)){turnLeft();} 
            //if Right Sensor and Left Sensor are at Black color then it will call Stop function
            if((digitalRead(R_S) == 1)&&(digitalRead(L_S) == 1)){Stop();}     
      } 
      delay(10);
}

#if WIFI == false
      void bt_receive(void){
            if(bluetooth.available()){   
                  recv_data = bluetooth.read();
                  Serial.println(recv_data);    
                  select_mode(); 
            }
      }

#else 
      void WifiConnection(void){
            // Configuration du mode AP (Point d'Accès)
            Serial.println("Configuration du point d'accès WiFi en mode AP : ");
            WiFi.mode(WIFI_AP);
            WiFi.softAPConfig(local_IP, gateway, subnet);
            WiFi.softAP(ssid_ap, password_ap);
            Serial.println("Point d'accès WiFi en mode AP configuré");
            Serial.print("Adresse IP AP: http://");
            Serial.println(WiFi.softAPIP());
            Serial.print("Adresse MAC AP: ");
            Serial.println(WiFi.softAPmacAddress());
            server.begin();
      }

      void WiFireceive(void){
            WiFiClient client = server.available();
            if(client) { 
                  if (client.connected()) {
                        if (client.available()) {
                              recv_data = client.parseInt();
                              Serial.println(recv_data);
                              select_mode();
                        }
                        delay(10);
                  }
                 
                  client.flush();
                  client.stop();
            }
      }
#endif


void select_mode(void){
      // Auto Line Follower Command
      if(recv_data == 8){
            Serial.println("Auto Line Follower Command");
            mode=1; 
            Speed=130;
      }   

      //Manual Android Application Control Command
      else if(recv_data == 9){
            Serial.println("Manual Android Application Control Command");
            mode=0; 
            Stop();
      } 
}

void forword(void){  //forword
      digitalWrite(in1, HIGH); //Right Motor forword Pin 
      digitalWrite(in2, LOW);  //Right Motor backword Pin 
      digitalWrite(in3, LOW);  //Left Motor backword Pin 
      digitalWrite(in4, HIGH); //Left Motor forword Pin 
}

void backword(void){ //backword
      digitalWrite(in1, LOW);  //Right Motor forword Pin 
      digitalWrite(in2, HIGH); //Right Motor backword Pin 
      digitalWrite(in3, HIGH); //Left Motor backword Pin 
      digitalWrite(in4, LOW);  //Left Motor forword Pin 
}

void turnRight(void){ //turnRight
      digitalWrite(in1, LOW);  //Right Motor forword Pin 
      digitalWrite(in2, LOW);  //Right Motor backword Pin   
      digitalWrite(in3, LOW);  //Left Motor backword Pin 
      digitalWrite(in4, HIGH); //Left Motor forword Pin 
}

void turnLeft(void){ //turnLeft
      digitalWrite(in1, HIGH);//Right Motor forword Pin 
      digitalWrite(in2, LOW); //Right Motor backword Pin 
      digitalWrite(in3, LOW); //Left Motor backword Pin 
      digitalWrite(in4, LOW); //Left Motor forword Pin  
}

void Stop(void){ //stop
      digitalWrite(in1, LOW); //Right Motor forword Pin 
      digitalWrite(in2, LOW); //Right Motor backword Pin 
      digitalWrite(in3, LOW); //Left Motor backword Pin 
      digitalWrite(in4, LOW); //Left Motor forword Pin 
}

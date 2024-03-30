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
 * 
 */
#ifdef __AVR__
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
      #include <BluetoothSerial.h>
      #include <WiFi.h>

      #if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
            #error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
      #endif

      BluetoothSerial bluetooth;

      #define LED_BUILTIN 2

      #define enA 15 // Enable1 L298 Pin enA 
      #define in1 4  // Motor1  L298 Pin in1 
      #define in2 16 // Motor1  L298 Pin in1 
      #define in3 17 // Motor2  L298 Pin in1 
      #define in4 5 // Motor2  L298 Pin in1 
      #define enB 18 // Enable2 L298 Pin enB
      #define R_S 19 // Capteur IR droit
      #define L_S 21 // Capteur IR gauche 

      // PWM Parameter
      #define FREQ        1000 // Channel frequency in Hz
      #define CHANNEL     0    // Channel number used
      #define RESOLUTION  8    // 8 bits (0 - 255)

      // Replace here with your WiFi SSID and password
      // WiFi access point to connect to
      const char* ssid     = "Nom_du_reseau_STA";
      const char* password = "Mot_de_passe_STA";

      // WiFi Access Point created
      const char* ssid_ap     = "Nom_du_reseau_AP"; // WiFi network name
      const char* password_ap = "Mot_de_passe_AP";  // WiFi network password

      void WifiConnection(void);

#else 
      #error "This code only works on an ESP 32 or Arduino board, select the correct board."
#endif

int bt_data;    // variable to receive data from the serial port
int Speed = 100; // The motor speed ranges from 0 to 255.  
int mode  = 0;

// Function prototypes
void bt_receive(void);
void select_mode(void);
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
            //Declare the pin associated with the ESP32 LED as an output and will indicate the Bluetooth state. 
            pinMode(LED_BUILTIN, OUTPUT);  
            digitalWrite(LED_BUILTIN, LOW);

            bluetooth.begin("MY_CAR"); // Bluetooth name
            WifiConnection(); // WiFi connection

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
      #ifdef ESP32
            if(bluetooth.connected()==true){
                  digitalWrite(LED_BUILTIN, HIGH); // Bluetooth ON
                  bt_receive();
            }
             else{
                  digitalWrite(LED_BUILTIN, LOW); // Bluetooth OFF
            }
            
            ledcWrite(CHANNEL, Speed);// Set the output signal of the channel
      
      #elif __AVR__
            bt_receive();
            analogWrite(enA, Speed); // Write The Duty Cycle 0 to 255 Enable Pin A for Motor1 Speed 
            analogWrite(enB, Speed); // Write The Duty Cycle 0 to 255 Enable Pin B for Motor2 Speed 
      #endif 

      if(mode==0){     
            //===============================================================================
            //                          Key Control Command
            //=============================================================================== 
            if(bt_data == 1){forword(); }  // if the bt_data is '1' the DC motor will go forward/Avant
            else if(bt_data == 2){backword();}  // if the bt_data is '2' the motor will Reverse/Aeeière
            else if(bt_data == 3){turnLeft();}  // if the bt_data is '3' the motor will turn left/Gauche
            else if(bt_data == 4){turnRight();} // if the bt_data is '4' the motor will turn right/Droite
            else if(bt_data == 5){Stop(); }     // if the bt_data '5' the motor will Stop/Stop

            //===============================================================================
            //                          Voice Control Command
            //===============================================================================    
            else if(bt_data == 6){turnLeft();  delay(400);  bt_data = 5;}
            else if(bt_data == 7){turnRight(); delay(400);  bt_data = 5;}
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


void bt_receive(void){
      if(bluetooth.available()){   
            bt_data = bluetooth.read();
            Serial.println(bt_data);    
            select_mode(); 
      }
}

void select_mode(void){
      // Auto Line Follower Command
      if(bt_data == 8){
            Serial.println("Auto Line Follower Command");
            mode=1; 
            Speed=130;
      }   

      //Manual Android Application Control Command
      else if(bt_data == 9){
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

#ifdef ESP32
  void WifiConnection(void){
              // Configuration du mode STA (Station)
              WiFi.mode(WIFI_STA);
              WiFi.begin(ssid, password);
              Serial.print("En cours de connexion au réseau WiFi en mode STA : ");
              int i=0;
              
              while (WiFi.status() != WL_CONNECTED){
                    ++i;
                    delay(1000);
                    Serial.print(".");
                    
                    if (i==5)
                          break;// Fin boucle
              }

              if (WiFi.status() == WL_CONNECTED){
                    Serial.println("\nConnecté au WiFi avec succès en mode STA !");
                    Serial.print("Adresse IP STA: http://");
                    Serial.println(WiFi.localIP());
                    Serial.print("Adresse MAC STA: ");
                    Serial.println(WiFi.macAddress());
              }
              else{
                    Serial.println("\nNon Connecté au WiFi en mode STA !");
              }

              // Configuration du mode AP (Point d'Accès)
              Serial.println("Configuration du point d'accès WiFi en mode AP : ");
              WiFi.softAP(ssid_ap, password_ap);
              Serial.println("Point d'accès WiFi en mode AP configuré");
              Serial.print("Adresse IP AP: http://");
              Serial.println(WiFi.softAPIP());
              Serial.print("Adresse MAC AP: ");
              Serial.println(WiFi.softAPmacAddress());
        }
#endif

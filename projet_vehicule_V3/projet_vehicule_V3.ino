/**
 * e-mail  : openprogramming23@gmail.com
 * Date    : 11/04/2024
 * Auteurs : @Exaucé KIMBEMBE / OpenProgramming
 *           @Ecrivez votre nom
 *  
 * @Carte : ESP32
 * 
 * Ce programme a été implémenté sur les cartes ESP32. 
 * Le programme est conçu pour créer un véhicule équipé d'un mode automatique 
 * (suiveur de ligne) et d'un mode manuel (contrôlé via Bluetooth).
 *
 */

// Définissez la constante WiFi sur false pour utiliser le Bluetooth et true pour le WiFi.
#define WIFI false 

#include <Wire.h>
#include "rgb_lcd.h"

rgb_lcd lcd;

#define colorR  255
#define colorG    0
#define colorB    0

#if WIFI == false
      #include <BluetoothSerial.h>

      #if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
            #error Le Bluetooth n'est pas activé ! Veuillez exécuter `make menuconfig` pour l'activer
      #endif

      BluetoothSerial bluetooth;

      // Prototypes de fonctions
      void bt_receive(void);

#else
      #include <WiFi.h>

      // Point d'accès WiFi créé
      const char* ssid_ap     = "WIFI_CAR"; // Nom du réseau WiFi
      const char* password_ap = "MON_PROJET_2024";  // Mot de passe du réseau WiFi

      IPAddress local_IP(192, 168, 4, 1); // Adresse IP statique pour le point d'accès WiFi
      IPAddress gateway(192, 168, 4, 1);  // Adresse IP de passerelle par défaut
      IPAddress subnet(255, 255, 255, 0); // Masque de sous-réseau

      WiFiServer server(80);

      // Prototypes de fonctions
      void WifiConnection(void);
      void WiFireceive(void);
#endif
      
#define LED_TEMOIN 2 // Indique l'état de connectivité
#define enA 15 // Broche de commande L298 enA 
#define in1 4  // Broche de moteur L298 in1 
#define in2 16 // Broche de moteur L298 in2 
#define in3 17 // Broche de moteur L298 in3 
#define in4 5  // Broche de moteur L298 in4 
#define enB 18 // Broche de commande L298 enB
#define R_S 32 // Capteur IR droit
#define L_S 33 // Capteur IR gauche 

// Paramètre PWM
#define FREQ        1000 // Fréquence du canal en Hz
#define CHANNEL     0    // Numéro de canal utilisé
#define RESOLUTION  8    // 8 bits (0 - 255)

int Speed = 100; // La vitesse du moteur varie de 0 à 255.  
int recv_data;   // Variable pour recevoir les données 
int mode  = 0;

// Prototypes de fonctions
void select_mode(void);
void forword(void);
void backword(void);
void turnRight(void);
void turnLeft(void);
void Stop(void);

void setup(){
      Serial.begin(115200);
      Serial.println("utilisation du moniteur série");

      // Configuration de l'écran LCD I2C Grove
      lcd.begin(16, 2);
      lcd.setRGB(colorR, colorG, colorB);
 
      // Configuration des broches
      pinMode(R_S, INPUT);  // Déclarer le capteur IR comme entrée  
      pinMode(L_S, INPUT);  // Déclarer le capteur IR comme entrée
      pinMode(enA, OUTPUT); // Déclarer la broche de commande L298 enA comme sortie
      pinMode(in1, OUTPUT); // Déclarer la broche de moteur L298 in1 comme sortie
      pinMode(in2, OUTPUT); // Déclarer la broche de moteur L298 in2 comme sortie
      pinMode(in3, OUTPUT); // Déclarer la broche de moteur L298 in3 comme sortie   
      pinMode(in4, OUTPUT); // Déclarer la broche de moteur L298 in4 comme sortie 
      pinMode(enB, OUTPUT); // Déclarer la broche de commande L298 enB comme sortie 

      #ifdef ESP32
            #if WIFI == false
                  bluetooth.begin("MY_CAR"); // Nom Bluetooth
            #else
                  WifiConnection();   // Connexion WiFi
            #endif
            pinMode(LED_TEMOIN, OUTPUT); 
            // Initialisation du canal PWM
            // Il est possible de connecter plusieurs broches au même canal PWM sur l'ESP32.
            ledcSetup(CHANNEL, FREQ, RESOLUTION); // Configurer le canal PWM
            ledcWrite(CHANNEL, Speed);   // Définir le signal de sortie du canal
            ledcAttachPin(enA, CHANNEL); // Attacher la broche au canal PWM
            ledcAttachPin(enB, CHANNEL); // Attacher la broche au canal PWM
      #endif
}

void loop(){  
      #if WIFI == false
            if(bluetooth.connected()==true){
                  digitalWrite(LED_TEMOIN,HIGH); // État allumé
                  bt_receive();
            }
            else{
                  digitalWrite(LED_TEMOIN,LOW); // État éteint
            }
                        
            ledcWrite(CHANNEL, Speed);// Définir le signal de sortie du canal
      #else
            WiFireceive();
            ledcWrite(CHANNEL, Speed);// Définir le signal de sortie du canal
      #endif

      if(mode==0){     
            //===============================================================================
            //                          Commande de contrôle manuel
            //===============================================================================
                  
            lcd.setCursor(0, 1);   
            lcd.print("Commande Manuel ");
            lcd.setCursor(0, 1);

            if(recv_data == 1){forword(); lcd.print("     AVANCE     ");}       // si recv_data est '1', le moteur DC avancera
            else if(recv_data == 2){backword(); lcd.print("     ARRIERE    ");} // si recv_data est '2', le moteur reculera
            else if(recv_data == 3){turnLeft(); lcd.print("TOURNER A GAUCHE");} // si recv_data est '3', le moteur tournera à gauche
            else if(recv_data == 4){turnRight();lcd.print("TOURNER A DROITE");} // si recv_data est '4', le moteur tournera à droite
            else if(recv_data == 5){Stop(); lcd.print("      STOP      ");}     // si recv_data est '5', le moteur s'arrêtera
            }
            else{    
            //===============================================================================
            //                          Contrôle du Suiveur de Ligne
            //===============================================================================   

            lcd.setCursor(0, 1);   
            lcd.print(" SUIVRE LA LIGNE ");
            lcd.setCursor(0, 1);   
            lcd.print("VITESSE : "+String(Speed)+"         ");

            // si le capteur droit et le capteur gauche sont blancs, on appelle la fonction forword
            if((digitalRead(R_S) == 0)&&(digitalRead(L_S) == 0)){forword();} 
            // si le capteur droit est noir et le capteur gauche est blanc, on appelle la fonction turn Right
            if((digitalRead(R_S) == 1)&&(digitalRead(L_S) == 0)){turnRight();} 
            // si le capteur droit est blanc et le capteur gauche est noir, on appelle la fonction turn Left
            if((digitalRead(R_S) == 0)&&(digitalRead(L_S) == 1)){turnLeft();} 
            // si le capteur droit et le capteur gauche sont noirs, on appelle la fonction Stop
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
                  digitalWrite(LED_TEMOIN,HIGH); // States on
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
            else{
                  digitalWrite(LED_TEMOIN,LOW); // States off
            }
      }
#endif

void select_mode(void){
      // Commande de suivi de ligne automatique
      if(recv_data == 8){
            Serial.println("Commande de suivi de ligne automatique");
            mode=1; 
            Speed=130;
      }   

      // Commande de contrôle manuel via l'application Android
      else if(recv_data == 9){
            Serial.println("Commande de contrôle manuel via l'application Android");
            mode=0; 
            Stop();
      } 
}

void forword(void){  //avancer
      digitalWrite(in1, HIGH); //Broche pour avancer du moteur droit
      digitalWrite(in2, LOW);  //Broche pour reculer du moteur droit
      digitalWrite(in3, LOW);  //Broche pour reculer du moteur gauche
      digitalWrite(in4, HIGH); //Broche pour avancer du moteur gauche
}

void backword(void){ //reculer
      digitalWrite(in1, LOW);  //Broche pour avancer du moteur droit
      digitalWrite(in2, HIGH); //Broche pour reculer du moteur droit
      digitalWrite(in3, HIGH); //Broche pour reculer du moteur gauche
      digitalWrite(in4, LOW);  //Broche pour avancer du moteur gauche
}

void turnRight(void){ //tourner à droite
      digitalWrite(in1, LOW);  //Broche pour avancer du moteur droit
      digitalWrite(in2, LOW);  //Broche pour reculer du moteur droit   
      digitalWrite(in3, LOW);  //Broche pour reculer du moteur gauche
      digitalWrite(in4, HIGH); //Broche pour avancer du moteur gauche
}

void turnLeft(void){ //tourner à gauche
      digitalWrite(in1, HIGH);//Broche pour avancer du moteur droit
      digitalWrite(in2, LOW); //Broche pour reculer du moteur droit
      digitalWrite(in3, LOW); //Broche pour reculer du moteur gauche
      digitalWrite(in4, LOW); //Broche pour avancer du moteur gauche  
}

void Stop(void){ //arrêter
      digitalWrite(in1, LOW); //Broche pour avancer du moteur droit
      digitalWrite(in2, LOW); //Broche pour reculer du moteur droit
      digitalWrite(in3, LOW); //Broche pour reculer du moteur gauche
      digitalWrite(in4, LOW); //Broche pour avancer du moteur gauche
}

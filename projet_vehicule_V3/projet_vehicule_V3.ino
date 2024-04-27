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

#define WIFI false 

#include <Wire.h>
#include "rgb_lcd.h"

#define nbreLigne    2  // Nombre des lignes de l'écran lcd
#define nbreColonne  16 // Nombre des colonnes de l'écran lcd

rgb_lcd lcd; // creation de la classe LCD

#if WIFI == false//si la connexion wifi nest pas activé ; c'est le bluetooth qui doit fonctionner
      #include <BluetoothSerial.h>// on appel la bibliotheque bluetooth qui est bluetoothSerial

      #if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)// verifier si la configuration du bluetooth de la carte ESP32 est activé ou disponible ou = || ;la configuration de la connexion du bluetooth de ESP32 avec le telephone androide est activé
            #error Le Bluetooth n'est pas activé ! Veuillez exécuter make menuconfig pour l'activer // si la condition précedente nest vrai ;afficher le message d'erreur
      #endif// #endif = fin de la condition commencer à la ligne 15

      BluetoothSerial bluetooth;//si la configuration decrite entre la ligne 15 et 17 est bonne ; creer l'objet "bluetooth" de la classe BluetoothSerial

      // Prototypes de fonctions
      void bt_receive(void); //  prototype de la fonction qui permet d'échanger les données entre le ESP32 et le télephone Androide

#else// si la connexion wifi sur la carte ESP32 est activé ; donc true à la ligne 1 ; ce qui veut dire qu'on va utiliser le connexion wifi
      #include <WiFi.h>// donc puisquon veut utiliser la connexion wifi ; on appelle la bibliothèque <WiFi.h>

      // Point d'accès WiFi créé
      const char* ssid_ap     = "WIFI_CAR"; // Nom du réseau WiFi
      const char* password_ap = "MON_PROJET_2024";  // Mot de passe du réseau WiFi

      IPAddress local_IP(192, 168, 4, 1); // Adresse IP statique pour le point d'accès WiFi
      IPAddress gateway(192, 168, 4, 1);  // Adresse IP de passerelle par défaut
      IPAddress subnet(255, 255, 255, 0); // Masque de sous-réseau

      WiFiServer server(80);// creation de l'objet server dans la classe WiFiServer

      // Prototypes de fonctions// Déclaration des deux fonction :void WifiConnection(void);et void WiFireceive(void);
      void WifiConnection(void);
      void WiFireceive(void);
#endif
      
#define LED_TEMOIN 2  // Indique l'état de connectivité
#define enA 15        // Broche de commande L298 enA 
#define in1 4         // Broche de moteur L298 in1 
#define in2 16        // Broche de moteur L298 in2 
#define in3 17        // Broche de moteur L298 in3 
#define in4 5         // Broche de moteur L298 in4 
#define enB 18        // Broche de commande L298 enB
#define R_S 32        // Capteur IR droit
#define L_S 34        // Capteur IR gauche 

// Paramètre PWM
#define FREQ        1000 // Fréquence du canal en Hz
#define CHANNEL     0    // Numéro de canal utilisé
#define RESOLUTION  8    // 8 bits (0 - 255)

int Speed = 100; // La vitesse du moteur varie de 0 à 255.  
int recv_data;   // Variable pour recevoir les données 
int mode = 2  ;  // declaration de la variable pour selectionner le mode manuel ou automatique

// Messages à afficher sur l'écran lcd I2C grove (maximums 16 caractère y compris les espaces)
const char MSG_ACCEUIL_1[nbreColonne+1]  = "VEUILLEZ CHOISIR";
const char MSG_ACCEUIL_2[nbreColonne+1]  = "MODE DE CONTROLE";
const char MSG_MANUEL[nbreColonne+1]     = "COMMANDE MANUEL ";
const char MSG_AUTO_1[nbreColonne+1]     = " ROBOT SUIVEUR  ";
const char MSG_AUTO_2[nbreColonne+1]     = " SUIVI DE LIGNE ";
const char MSG_AVANCE[nbreColonne+1]     = "     AVANCE     ";
const char MSG_ARRIERE[nbreColonne+1]    = "     ARRIERE    ";
const char MSG_GAUCHE[nbreColonne+1]     = "TOURNER A GAUCHE";
const char MSG_DROITE[nbreColonne+1]     = "TOURNER A DROITE";
const char MSG_STOP  [nbreColonne+1]     = "      STOP      ";

// Prototypes de fonctions// Déclaration des fonction definit en bas 
void affichage_lcd(const char texte_1[], const char texte_2[]); // Permet d'afficher le message sur l'écran lcd I2C grove
void select_mode(void);//fonction qui permet de selectionner le mode manuelle ou automatique
void forword(void);
void backword(void);
void turnRight(void);
void turnLeft(void);
void Stop(void);

void setup(){
      Serial.begin(115200);// vitesse de communication du moniteur serie

      // Configuration de l'écran LCD I2C Grove
      Wire.begin(); // Initialisation de la communication I2C pour l'ecran LCD I2C grove
      lcd.begin(nbreColonne, nbreLigne);// taille de lecran ; nombre collonnes = 16 et nombre de ligne = 2
      affichage_lcd(MSG_ACCEUIL_1, MSG_ACCEUIL_2); // Affichage du message sur les deux lignes de l'écran 

      // Configuration des broches pour dine si le signal est entrant ou sortant dans la fonction pinMode();
      //La fonction pinMode(); prend toujours deux argument : nom de la broche et le type de signal si entrant ou sortant
      pinMode(R_S, INPUT);  // Déclarer le capteur IR droit comme entrée  
      pinMode(L_S, INPUT);  // Déclarer le capteur IR gauche comme entrée
      pinMode(enA, OUTPUT); // Déclarer la broche de commande L298 enA comme sortie
      pinMode(in1, OUTPUT); // Déclarer la broche de moteur L298 in1 comme sortie
      pinMode(in2, OUTPUT); // Déclarer la broche de moteur L298 in2 comme sortie
      pinMode(in3, OUTPUT); // Déclarer la broche de moteur L298 in3 comme sortie   
      pinMode(in4, OUTPUT); // Déclarer la broche de moteur L298 in4 comme sortie 
      pinMode(enB, OUTPUT); // Déclarer la broche de commande L298 enB comme sortie 

      #if WIFI == false //on reverifie dans le void setup ,si doit activé le wifi ou le bluetooth de ESP32
            bluetooth.begin("MY_CAR"); // Activation de la connexion Bluetooth ("MY_CAR" est le nom du bluetooth)
      #else
            WifiConnection();   // Activation de la connexion WiFi
      #endif
            
      pinMode(LED_TEMOIN, OUTPUT); //definition du mode entree sortie de la LED temoign de connexion
      // Initialisation du canal PWM
      // Il est possible de connecter plusieurs broches au même canal PWM sur l'ESP32.
      ledcSetup(CHANNEL, FREQ, RESOLUTION); // Configurer le canal PWM//  chercher chacune des quatre fonction en rouge
      ledcWrite(CHANNEL, Speed);   // Définir le signal de sortie du canal
      ledcAttachPin(enA, CHANNEL); // Attacher le moteur A au canal PWM
      ledcAttachPin(enB, CHANNEL); // Attacher le moteur B au canal PWM
}


void loop() {  
    #if WIFI == false
        if(bluetooth.connected()) {// verifier si le ESP32 est connecté au bluetooth
            digitalWrite(LED_TEMOIN, HIGH); // État allumé
            bt_receive();// fonction qui permet de recevoir les données transmises par l'application//
        } else {
            digitalWrite(LED_TEMOIN, LOW); // État éteint
        }
                    
        ledcWrite(CHANNEL, Speed); // Définir le signal de sortie du canal// cette fonction permet de modifier la vitesse des moteurs depuis lapplication 
    #else
        WiFireceive();//fonction qui permet de recevoir des donées via wifi , lorsque cest conecté à lautre carte ESP32
        ledcWrite(CHANNEL, Speed); // Définir le signal de sortie du canal , definir le CHANNEL et Speed pour la mode de connexion wifi
    #endif

    // Par défaut la variable mode est initialisé a 2 de telle sorte que ni Commande de contrôle manuel ou suivi de la ligne est activé
    if(mode == 0) { // mode comprend deux valeur qui sont 0 ou 1, et ces deux valeurs sont definit dans la fonction select_mode();
        //===============================================================================
        //                          Commande de contrôle manuel
        //=============================================================================== 
        if(recv_data == 1) {
            forword(); 
            affichage_lcd(MSG_MANUEL, MSG_AVANCE); // Affichage du message sur les deux lignes de l'écran 
        } else if(recv_data == 2) {
            backword(); 
            affichage_lcd(MSG_MANUEL, MSG_ARRIERE); // Affichage du message sur les deux lignes de l'écran 
        } else if(recv_data == 3) {
            turnLeft(); 
            affichage_lcd(MSG_MANUEL, MSG_GAUCHE); // Affichage du message sur les deux lignes de l'écran 
        } else if(recv_data == 4) {
            turnRight();
            affichage_lcd(MSG_MANUEL, MSG_DROITE); // Affichage du message sur les deux lignes de l'écran 
        } else if(recv_data == 5) {
            Stop(); 
            affichage_lcd(MSG_MANUEL, MSG_STOP); // Affichage du message sur les deux lignes de l'écran 
        }
    } 
    else if (mode == 1) {  // si la variable mode est égale a 1, la suivi de ligne par les capteurs ir est activé 
        //===============================================================================
        //                          Contrôle du Suiveur de Ligne
        //===============================================================================   
        // si le capteur droit et le capteur gauche sont blancs, on appelle la fonction forword
        if((digitalRead(R_S) == 0) && (digitalRead(L_S) == 0)) {
            forword();
        } 
        // si le capteur droit est noir et le capteur gauche est blanc, on appelle la fonction turn Right
        else if((digitalRead(R_S) == 1) && (digitalRead(L_S) == 0)) {
            turnRight();
        } 
        // si le capteur droit est blanc et le capteur gauche est noir, on appelle la fonction turn Left
        else if((digitalRead(R_S) == 0) && (digitalRead(L_S) == 1)) {
            turnLeft();
        } 
        // si le capteur droit et le capteur gauche sont noirs, on appelle la fonction Stop
        else if((digitalRead(R_S) == 1) && (digitalRead(L_S) == 1)) {
            Stop();
        } else {
            Stop(); // Arrêter par défaut si aucune condition n'est remplie
        }
    } 
    delay(10); // La finction loop() se répète après un temps de pause 10 millisecondes
}//fin de la fonction void loop() qui tourne en boucle // cest la fonction principale


#if WIFI == false
      void bt_receive(void){//Definition de la fonction qui permet de recevoir les donnees envoyé par App inventor
            if(bluetooth.available()){// on verifie si le bluetooth est active en appellant la fonction available(); qui se trouve dans la bibliotheque :<BluetoothSerial.h> :afin de recevoir les données envoyé par App inventor   
                  recv_data = bluetooth.read(); //affectation à la varriable recv_data de la valeur lu par le bluetooth envoyé depuis App inventor
                  select_mode(); //fonction permettant de basculer entre le mode manuel et automatique
            }
      }//fin de la fonction void bt_receive(void)
#else 
      void WifiConnection(void){
            // Configuration du mode AP (Point d'Accès)
            Serial.println("Configuration du point d'accès WiFi en mode AP : "); // Affiche un message sur le moniteur série
            WiFi.mode(WIFI_AP); // Configure le WiFi en mode Point d'Accès (AP)
            WiFi.softAPConfig(local_IP, gateway, subnet); // Configure l'adresse IP, la passerelle et le masque de sous-réseau du point d'accès
            WiFi.softAP(ssid_ap, password_ap); // Configure le nom et le mot de passe du réseau WiFi du point d'accès
            Serial.println("Point d'accès WiFi en mode AP configuré"); // Affiche un message sur le moniteur série
            Serial.print("Adresse IP AP: http://"); // Affiche un message sur le moniteur série
            Serial.println(WiFi.softAPIP()); // Affiche l'adresse IP du point d'accès sur le moniteur série
            Serial.print("Adresse MAC AP: "); // Affiche un message sur le moniteur série
            Serial.println(WiFi.softAPmacAddress()); // Affiche l'adresse MAC du point d'accès sur le moniteur série
            server.begin(); // Démarre le serveur
      }

      void WiFireceive(void){// la fonction qui permet la reception des donnees via la connexion wifi
            WiFiClient client = server.available();//creation de lobjet client de la classe WiFiClient et initialisation de lobjet client a : server.available();
            if(client) { // Vérifie s'il y a un client WiFi connecté
                  digitalWrite(LED_TEMOIN,HIGH); // Allume la LED témoin pour indiquer que le client est connecté
                  if (client.connected()) {// Vérifie si le client est toujours connecté a l'ESP32(deux carte ESP32 sont connecté)
                        if (client.available()) {//verifier si lappareil connecté est disponible ou activé
                              recv_data = client.parseInt();//Lorsque on recoit la veleur envoyé par la telecommande ; on la stock dans la variable :recv_data ; grace a la fonction parseInt() .Cette fonction est lequivqlente de la read();pour le fonctionnement Bluetooth// ces deux fonction permet de recuperer et lire les donnees envoyées
                              Serial.println(recv_data);// Affiche les données reçues sur le moniteur série
                              select_mode();//fonction permettant de basculer entre le mode manuel et automatique
                        }
                        delay(10);//ce delai permet de verifier tous les 10 millisecondes ; si dejà connecté est toujour disponible
                  }
                 
                  client.flush(); // Vide le tampon de réception du client
                  client.stop();  // Ferme la connexion avec le client
            }
            else{//si le ESP 32 n'est pas connecté a un appareil 
                  digitalWrite(LED_TEMOIN,LOW); // Éteint la LED témoin pour indiquer qu'aucun client n'est connecté
            }
      }
#endif

void select_mode(void){
      // Commande de suivi de ligne automatique
      if(recv_data == 8){
            mode=1; // si on appuie sur 8 == mode automatique activé; on passe la variable à 1
            Speed=130;//vitesse a la quelle le robot va suivre la ligne 
            affichage_lcd(MSG_AUTO_1, MSG_AUTO_2); // Affichage du message sur les deux lignes de l'écran 
      }   
      // Commande de contrôle manuel via l'application Android
      else if(recv_data == 9){
            mode=0; // si on appuie sur 9 == mode manuel activé; on passe la variable à 0
            Stop();//on arrete le robot pour attendre les commande ou instruction de la commande manuelle
            affichage_lcd(MSG_MANUEL, MSG_STOP); // Affichage du message sur les deux lignes de l'écran
      } 
}

/**
 * Permet d'afficher le message sur l'écran lcd I2C grove
 * paramètres :
 *    char texte_1[] : tableau contenant le message a afficher sur la ligne 1
 *    char texte_2[] : tableau contenant le message a afficher sur la ligne 2
*/
void affichage_lcd(const char texte_1[], const char texte_2[]){
      for (int i = 0; i <= 1; i++) { // Permet de choisir le message a afficher sur la ligne
            for (int ligne = 0; ligne < nbreLigne; ligne++) { // Parcour de la ligne
                  for (int col = 0; col < nbreColonne; col++) { // Parcour de la colonne
                        lcd.setCursor(col, ligne); // Positionnement du curseur
                        if(i==0 and ligne == 0)lcd.write(texte_1[col]); // Affichage du message ligne 1
                        else if(i==1 and ligne == 1)lcd.write(texte_2[col]); // Affichage du message ligne 2
                  }
            }
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

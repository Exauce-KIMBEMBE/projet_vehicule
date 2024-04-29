/**
 * e-mail  : openprogramming23@gmail.com
 * Date    : 06/04/2024
 * Auteurs : @Exaucé KIMBEMBE / OpenProgramming
 *        
 *  
 * @Board : ESP32
 * 
 * Ce programme gère le fonctionnement d'une télécommande conçue pour gérer un véhicule via un réseau Wi-Fi. 
 * Le code se connecte à un serveur établi par une carte ESP32 montée sur le véhicule et établit 
 * une connexion Wi-Fi en mode point d'accès (AP). 
 *  @@ Les matériaux nécessaires pour exécuter le programme comprennent :
 *    ** Une carte ESP32
 *    ** Un joystick
 *    ** Deux boutons-poussoirs
 *    ** Un écran LCD GROVE I2C
 *    ** 5 LEDs
 * 
 */

// Permet de vérifier que l'on est bien sur esp32
#ifndef ESP32
  #error "Ce code a été testé sur une carte ESP32." 
#endif

#include <WiFi.h>
#include <Wire.h>
#include <rgb_lcd.h>

#define nbreLigne    2  // Nombre des lignes de l'écran lcd
#define nbreColonne  16 // Nombre des colonnes de l'écran lcd

// Broches utilisées pour le joystick
#define RX   32 // Contrôle les mouvements selon RX (AVANT ET ARRIÈRE)
#define RY   33 // Contrôle les mouvements selon RY (GAUCHE ET DROITE)
// Broches utilisées par les LED pour indiquer la direction de la voiture
#define LED_R 13  // Indique le virage à droite
#define LED_L 14  // Indique le virage à gauche
#define LED_F 18  // Indique le mouvement vers l'avant
#define LED_B 23  // Indique le mouvement vers l'arrière
#define LED_S 15  // Indique l'état du wifi si connecté ou pas
// Broches utilisées par les boutons
#define BTN_MANUEL   19 // Bouton de contrôle manuel
#define BTN_AUTO     12 // Bouton de contrôle suiveur de ligne

// Constantes
#define DEFAULT     50 // Valeur du joystick au repos 
#define IT          30 // (IT = intervalle) envoi des données que si la valeur du joystick est :  DEFAULT + ou - IT 

// Valeurs à envoyer en fonction de la position du joystick
#define FORWARD     1  // Avance
#define BACKWARD    2  // Arrière
#define LEFT        3  // Gauche
#define RIGHT       4  // Droite
#define STOP        5  // Arrêt
#define AUTO_MODE   8  // Suiveur ligne
#define MANUEL_MODE 9  // Commande manuel

// Messages à afficher sur l'écran lcd I2C grove (maximums 16 caractère y compris les espaces)
const char MSG_ACCEUIL_1[nbreColonne+1]      = "VEUILLEZ CHOISIR";
const char MSG_ACCEUIL_2[nbreColonne+1]      = "MODE DE CONTROLE";
const char MSG_MANUEL[nbreColonne+1]         = "COMMANDE MANUEL ";
const char MSG_AUTO_1[nbreColonne+1]         = " ROBOT SUIVEUR  ";
const char MSG_AUTO_2[nbreColonne+1]         = " SUIVI DE LIGNE ";
const char MSG_AVANCE[nbreColonne+1]         = "     AVANCE     ";
const char MSG_ARRIERE[nbreColonne+1]        = "     ARRIERE    ";
const char MSG_GAUCHE[nbreColonne+1]         = "TOURNER A GAUCHE";
const char MSG_DROITE[nbreColonne+1]         = "TOURNER A DROITE";
const char MSG_STOP  [nbreColonne+1]         = "      STOP      ";
const char MSG_ERROR_WIFI_1[nbreColonne+1]   = " CONNEXION WIFI ";
const char MSG_ERROR_WIFI_2[nbreColonne+1]   = "     PERDUE     ";
const char MSG_ERROR_SERVER_1[nbreColonne+1] = "ERREUR TRANSFERT";
const char MSG_ERROR_SERVER_2[nbreColonne+1] = "   DES DONNEES  ";

const char* ssid     = "WIFI_CAR";        // Nom du WIFI
const char* password = "MON_PROJET_2024"; // Mot de passe WIFI
const int serverPort = 80; // Port du serveur

// Déclaration des objets
rgb_lcd lcd; // creation de la classe LCD
WiFiClient client; // Client WiFi pour la communication avec le serveur
IPAddress serverIP(192, 168, 4, 1); // Adresse IP du serveur

// prototypes des fonctions
void ledOff(void);
void sendData(void);


void setup() {
  Serial.begin(115200); // Démarre la communication série à une vitesse de 115200 bauds

  // Configuration de l'écran LCD I2C Grove
  Wire.begin(); // Initialisation de la communication I2C pour l'ecran LCD I2C grove
  lcd.begin(nbreColonne, nbreLigne);// taille de lecran ; nombre collonnes = 16 et nombre de ligne = 2
  affichage_lcd(MSG_ACCEUIL_1, MSG_ACCEUIL_2); // Affichage du message sur les deux lignes de l'écran 

  // Configuration des broches
  pinMode(RX,INPUT); // Configure la broche RX en entrée pour le joystick
  pinMode(RY,INPUT); // Configure la broche RY en entrée pour le joystick
  pinMode(LED_R, OUTPUT); // Configure la broche LED_R en sortie pour la LED indiquant le virage à droite
  pinMode(LED_L, OUTPUT); // Configure la broche LED_L en sortie pour la LED indiquant le virage à gauche
  pinMode(LED_F, OUTPUT); // Configure la broche LED_F en sortie pour la LED indiquant le mouvement vers l'avant
  pinMode(LED_B, OUTPUT); // Configure la broche LED_B en sortie pour la LED indiquant le mouvement vers l'arrière
  pinMode(LED_S, OUTPUT); // Configure la broche LED_S en sortie pour la LED indiquant l'état du WiFi
  pinMode(BTN_AUTO, INPUT_PULLUP); // Configure la broche BTN_AUTO en entrée avec résistance de tirage vers le haut
  pinMode(BTN_MANUEL, INPUT_PULLUP); // Configure la broche BTN_MANUEL en entrée avec résistance de tirage vers le haut
  
  digitalWrite(LED_S, LOW); // Éteint la LED indiquant l'état du WiFi

  // Connexion au réseau WiFi avec l'adresse MAC spécifiée
  Serial.println("Connexion au réseau WiFi en cours...");
  WiFi.mode(WIFI_STA); // Configure le mode WiFi en mode station (STA)
  WiFi.begin(ssid,password); // Démarre la connexion au réseau WiFi avec les identifiants spécifiés
  
  while (WiFi.status() != WL_CONNECTED) { // Boucle jusqu'à ce que la connexion au réseau WiFi soit établie
      Serial.print("."); // Affiche un point pour indiquer que la connexion est en cours
      delay(500); // Attend 500 millisecondes
    }

  Serial.println("Connecté"); // Affiche "Connecté" une fois la connexion au réseau WiFi établie
  digitalWrite(LED_S, HIGH); // Allume la LED indiquant l'état du WiFi
}


void loop() {
  
  if (WiFi.status() == WL_CONNECTED) { // Vérifie si la connexion WiFi est établie

    if (client.connect(serverIP, serverPort)){ // Si la connexion avec le serveur est établie
      // Activation du mode manuel
      if (not digitalRead(BTN_MANUEL)==true){
        client.println(MANUEL_MODE); // Envoie la commande de mode manuel au serveur
        client.stop(); // Arrête la connexion client
        affichage_lcd(MSG_AUTO_1, MSG_AUTO_2); // Affichage du message sur les deux lignes de l'écran        
        delay(100); // Attend 100 millisecondes
        return; // Sort de la fonction loop()
      }
      // Activation du mode suiveur de ligne
      else if (not digitalRead(BTN_AUTO)==true){
        client.println(AUTO_MODE); // Envoie la commande de mode suiveur de ligne au serveur
        client.stop(); // Arrête la connexion client
        Serial.println(MANUEL_MODE); // Affiche "MANUEL_MODE" sur le moniteur série
        affichage_lcd(MSG_MANUEL, MSG_STOP); // Affichage du message sur les deux lignes de l'écran
        delay(100); // Attend 100 millisecondes
        return; // Sort de la fonction loop()
      }
      else{ // Transmission des données du joystick
        sendData(); // Appelle la fonction pour envoyer les données de contrôle au serveur
        client.stop(); // Arrête la connexion client
      }
    } 
    else {// Affiche un message d'erreur si la connexion au serveur a échoué
      affichage_lcd(MSG_ERROR_SERVER_1, MSG_ERROR_SERVER_2); // Affichage du message sur les deux lignes de l'écran
    }

    delay(10); // Attend 10 millisecondes
  } 
  else {// Affiche un message d'erreur si la connexion wifi est perdue
    affichage_lcd(MSG_ERROR_WIFI_1, MSG_ERROR_WIFI_2); // Affichage du message sur les deux lignes de l'écran
    digitalWrite(LED_S, LOW); // Éteint la LED indiquant l'état du WiFi
    delay(3000); // Attend 3 secondes
    ESP.restart(); // Redémarre le module ESP32
  }
}


void sendData(void){
  int rx_value = map(analogRead(RX),0,4095,0,100); // Lecture de la valeur du joystick en X et mise à l'échelle de 0 à 100
  int ry_value = map(analogRead(RY),0,4095,0,100); // Lecture de la valeur du joystick en Y et mise à l'échelle de 0 à 100

  // ARRÊT
  if((rx_value >=(DEFAULT-IT) and rx_value <= (DEFAULT+IT)) and (ry_value >= (DEFAULT-IT) and ry_value <= (DEFAULT+IT))){
    client.println(STOP); // Envoie la commande d'arrêt au serveur
    affichage_lcd(MSG_MANUEL, MSG_STOP); // Affichage du message sur les deux lignes de l'écran
    ledOff(); // Éteint toutes les LEDs
  }
  // AVANT
  else if(rx_value > (DEFAULT+IT)){
    client.println(FORWARD); // Envoie la commande d'avancer au serveur
    affichage_lcd(MSG_MANUEL, MSG_AVANCE); // Affichage du message sur les deux lignes de l'écran 
    ledOff(); // Éteint toutes les LEDs
    digitalWrite(LED_F,HIGH); // Allume la LED correspondante au mouvement vers l'avant
  }
  // ARRIÈRE
  else if(rx_value < (DEFAULT-IT)){
    client.println(BACKWARD); // Envoie la commande de reculer au serveur
    affichage_lcd(MSG_MANUEL, MSG_ARRIERE); // Affichage du message sur les deux lignes de l'écran
    ledOff(); // Éteint toutes les LEDs
    digitalWrite(LED_B,HIGH); // Allume la LED correspondante au mouvement vers l'arrière
  }
  // GAUCHE
  else if(ry_value < (DEFAULT-IT)){
    client.println(LEFT); // Envoie la commande de tourner à gauche au serveur
    affichage_lcd(MSG_MANUEL, MSG_GAUCHE); // Affichage du message sur les deux lignes de l'écran 
    ledOff(); // Éteint toutes les LEDs
    digitalWrite(LED_L,HIGH); // Allume la LED correspondante au virage à gauche
  }
  // DROITE
  else if(ry_value > (DEFAULT+IT)){
    client.println(RIGHT); // Envoie la commande de tourner à droite au serveur
    affichage_lcd(MSG_MANUEL, MSG_DROITE); // Affichage du message sur les deux lignes de l'écran
    ledOff(); // Éteint toutes les LEDs
    digitalWrite(LED_R,HIGH); // Allume la LED correspondante au virage à droite
  }
}


void ledOff(void){
  digitalWrite(LED_F,LOW);
  digitalWrite(LED_B,LOW);
  digitalWrite(LED_L,LOW);
  digitalWrite(LED_R,LOW);
  delay(2);
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

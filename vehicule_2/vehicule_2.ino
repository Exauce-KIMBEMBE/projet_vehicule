#include <SoftwareSerial.h>
#include <Servo.h>

SoftwareSerial BT_Serial(2, 3); // RX, TX

#define enA 5//Enable1 L298 Pin enA 
#define in1 8 //Motor1  L298 Pin in1 
#define in2 9 //Motor1  L298 Pin in1 
#define in3 10 //Motor2  L298 Pin in1 
#define in4 11 //Motor2  L298 Pin in1 
#define enB 6 //Enable2 L298 Pin enB

#define R_S A0 //Capteur IR droit
#define L_S A1 //Capteur IR gauche 

//Ultrason
#define echo A2//Reception du siganl
#define trigger A3 //Envoie du siganl
#define pin_servo A4
int Set=15;//Position du servo moteur
int distance_L, distance_F, distance_R; //distance gauche, face et droite

int bt_data; // Variable pour stocker la valeur reçu par le Bluetooth
int Speed = 130;  //Vitesse des moteurs

int mode=0;

Servo servo; // Objet Servo

// Prototypes des fonctions
int  Ultrasonic_read(void);
void compareDistance(void);
void Check_side(void);
void forword(void);
void backword(void);
void turnRight(void);
void turnLeft(void);
void Stop(void);


void setup(){
    Serial.begin(9600); // Vitesse de communication 
    BT_Serial.begin(9600); 
    
    servo.attach(pin_servo);
    servo.write(70);

    //Déclaration des infrarouges en entrées 
    pinMode(R_S, INPUT); 
    pinMode(L_S, INPUT);

    //Déclaration des broches des moteurs en sorties
    pinMode(enA, OUTPUT);  //L298 Pin enA 
    pinMode(in1, OUTPUT);  //L298 Pin in1 
    pinMode(in2, OUTPUT);  //L298 Pin in2 
    pinMode(in3, OUTPUT);  //L298 Pin in3   
    pinMode(in4, OUTPUT);  //L298 Pin in4 
    pinMode(enB, OUTPUT);  //L298 Pin enB 

    //Début télémètre
    int attente = 10;
    for (int angle = 70; angle <= 140; angle += 5){
       servo.write(angle); 
       delay(attente);
    }
    for (int angle = 140; angle >= 0; angle -= 5){
       servo.write(angle); 
       delay(attente);
    }
    for (int angle = 0; angle <= 70; angle += 5){
       servo.write(angle); 
       delay(attente);
    }
     
    distance_F = Ultrasonic_read();
    delay(500);
    //Fin télémètre
}

void loop(){  
    if(BT_Serial.available() > 0){  
        bt_data = BT_Serial.read();      
        if(bt_data > 20){Speed = bt_data;}      
    }

    if(bt_data == 8){mode=1; Speed=50;} //Suivi automatique de lignes
    else if(bt_data == 9){mode=0; Stop();} //Commande manuelle via Android Application

    analogWrite(enA, Speed); 
    analogWrite(enB, Speed);

    if(mode==0){     
        //===============================================================================
        //                          Commande manuelle 
        //=============================================================================== 
        if(bt_data == 1){forword();}  // si la valeur reçue par le bluetooth est "1", le robot avance
        else if(bt_data == 2){backword();}  // si la valeur reçue par le bluetooth est "2", le robot fait marche arrière
        else if(bt_data == 3){turnLeft();}  // si la valeur reçue par le bluetooth est "3", le robot tourne à gauche
        else if(bt_data == 4){turnRight();} // si la valeur reçue par le bluetooth est "4", le robot tourne à droite
        else if(bt_data == 5){Stop(); }     // si la valeur reçue par le bluetooth est "5", le robot s'arrête

        //======================================================================================
        //Commande Ultrason:vérification de la présence d'obstacle et recherche du chémin libre
        //======================================================================================   
        else if(bt_data == 6){turnLeft();  delay(400);  bt_data = 5;}
        else if(bt_data == 7){turnRight(); delay(400);  bt_data = 5;}
    }
    else{    
        //===============================================================================
        //                           Control suivi de lignes
        //===============================================================================  

        distance_F = Ultrasonic_read();
        Serial.print("D F=");Serial.println(distance_F);
        
        //if Right Sensor and Left Sensor are at White color then it will call forword function
        if((digitalRead(R_S) == 0)&&(digitalRead(L_S) == 0)){
            if(distance_F > Set){forword();}
            else{Check_side();}  
        }  
        //if Right Sensor is Black and Left Sensor is White then it will call turn Right function
        else if((digitalRead(R_S) == 1)&&(digitalRead(L_S) == 0)){turnRight();}  
        //if Right Sensor is White and Left Sensor is Black then it will call turn Left function
        else if((digitalRead(R_S) == 0)&&(digitalRead(L_S) == 1)){turnLeft();} 

        /*if((digitalRead(R_S) == 0)&&(digitalRead(L_S) == 0)){forword();} 
        if((digitalRead(R_S) == 1)&&(digitalRead(L_S) == 0)){turnRight();}
        if((digitalRead(R_S) == 0)&&(digitalRead(L_S) == 1)){turnLeft();} 
        if((digitalRead(R_S) == 1)&&(digitalRead(L_S) == 1)){Stop();}*/
        delay(10);
    }
}


//Fonction du capteur Ultrason********
int Ultrasonic_read(void){
    digitalWrite(trigger, LOW);
    delayMicroseconds(2);
    digitalWrite(trigger, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigger, LOW);

    unsigned long time = pulseIn (echo, HIGH);
    return time* 0.034 / 2;//On divise par 2 pour trouver le temps parcouru par l'onde entre le robot et l'obstacle puisque 'time' correspond au temps d'aller-retour
}

void compareDistance(void){
    if(distance_L > distance_R){
        turnLeft();
        delay(500);
        forword();
        delay(600);
        turnRight();
        delay(500);
        forword();
        delay(600);
        turnRight();
        delay(400);
    }
    else{
        turnRight();
        delay(500);
        forword();
        delay(600);
        turnLeft();
        delay(500);
        forword();
        delay(600);  
        turnLeft();
        delay(400);
    }
}

void Check_side(void){
    Stop();
    delay(100);
 
    for (int angle = 70; angle <= 140; angle += 5){
        servo.write(angle); 
    }

    delay(300);
    distance_R = Ultrasonic_read();
    Serial.print("D R=");Serial.println(distance_R);
    delay(100);
  
    for (int angle = 140; angle >= 0; angle -= 5)  {
       servo.write(angle); 
    }
  
    delay(500);
    distance_L = Ultrasonic_read();
    Serial.print("D L=");Serial.println(distance_L);
    delay(100);
 
    for (int angle = 0; angle <= 70; angle += 5)  {
       servo.write(angle); 
    }

    delay(300);
    compareDistance();
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
    digitalWrite(in2, HIGH); //Right Motor backword Pin  
    digitalWrite(in3, LOW);  //Left Motor backword Pin 
    digitalWrite(in4, HIGH); //Left Motor forword Pin 
}

void turnLeft(void){ //turnLeft
    digitalWrite(in1, HIGH); //Right Motor forword Pin 
    digitalWrite(in2, LOW);  //Right Motor backword Pin 
    digitalWrite(in3, HIGH); //Left Motor backword Pin 
    digitalWrite(in4, LOW);  //Left Motor forword Pin 
}

void Stop(void){ //stop
    digitalWrite(in1, LOW); //Right Motor forword Pin 
    digitalWrite(in2, LOW); //Right Motor backword Pin 
    digitalWrite(in3, LOW); //Left Motor backword Pin 
    digitalWrite(in4, LOW); //Left Motor forword Pin 
}
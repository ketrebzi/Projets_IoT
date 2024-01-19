#include <SoftwareSerial.h>
#include <DHT.h>
//-----------------------------------------------------------------------
//CODE FINAL
#define RX 10
#define TX 11
String AP = "condor A8";//"AndroidAP";       // CHANGE ME
String PASS = "azerty2020";//"fer19946"; // CHANGE ME
String API = "QFP9UBUZJJCVZMG4";   // CHANGE ME
String HOST = "api.thingspeak.com";
String PORT = "80";
String field1 = "field1";
String field2 = "field2";
String field3 = "field3";
String field4 = "field4";
String field5 = "field5";
int countTrueCommand;
int countTimeCommand; 
boolean found = false; 
int valSensor = 1;
SoftwareSerial esp8266(RX,TX); 
//##########################################################
#define DHT01PIN A0     // la pin de DHT11 INTERIEUR
#define DHT02PIN A1     // la pin de DHT11 EXTERIEUR
#define DHTTYPE DHT11   // DHT 11
DHT dht01(DHT01PIN, DHTTYPE);
DHT dht02(DHT02PIN, DHTTYPE);

int CapteurLUM=A2,CapteurFLAM=A3;
int FLAM_MAX=70,LUMUNOSITE_MAX=512;
float TEMPERATURE01=0,HUMIDITE01=0,TEMPERATURE02=0,HUMIDITE02=0,LUMUNOSITE=0,FLAM=0,MOVEMENT=0;
String MSGC="";
int FCoursS=A5,FCoursT=A4;
int EtatFCT=false,EtatFCS=false;
int Buzzer=3,CapteurMOV=2;
int  ALARME=0;
int CMP_Buzzer=0;
bool RESULTAT=false;
//-------------------------------------
long DelaisEnvoiServeur=10000;
long DelaisEnvoiSMS_FLAM=10000,DelaisEnvoiSMS_S=10000,DelaisEnvoiSMS_T=10000;
long ServeurSave=0;
long SMS_FLAM_Save=0;
long SMS_cptT_Save=0;
long SMS_cptS_Save=0,SMS_TMP=0,DelaisEnvoiSMS_TMP=0,SMS_TMP1=0,DelaisEnvoiSMS_TMP1=0;
String MSG_FLAM="ATTENTION DANGER FLAMMES";
String MSG_VOLEUR2="ATTENTION LA RUCHE EST ENLEVE";
String MSG_VOLEUR1="ATTENTION LE TOIT DE LA RUCHE EST ENLEVE";
String MSG_TMP="ATTENTION TEMPERATURE  ELEVE";
String MSG_TMP1="ATTENTION TEMPERATURE  BAISSE";
String VOTRE_NUMERO="+213798242919";

//String MSG_HUM_MAX="ATTENTION HUMIDITE  ELEVE";
//String MSG_HUM_MIN="ATTENTION HUMIDITE  BAISSE";
//String MSG_MOUVEMENT="ATTENTION ";
long SMS_MOUVEMENT=0,DelaisEnvoiSMS_MOUVEMENT=0,SMS_HUM_MAX=0,DelaisEnvoiSMS_HUM_MAX=0,SMS_HUM_MIN=0,DelaisEnvoiSMS_HUM_MIN=0;
int HUM_MAX=55,HUM_MIN=20;
//#####################################
  void setup(){
    Serial.begin(9600);//moniture série
    delay(2000);
    String CMD="";
    CMD="AT";
    CMD=CMD+"\r";
    Serial.print(CMD);         // initialoisation de module GSM SIM900
    delay(1000); 
    //SIM900_EMETEUR_MSG("DHHHHHH");
    dht01.begin();//capteur DHT11 Activé
     dht02.begin();//capteur DHT11 Activé
    pinMode(FCoursT,INPUT); pinMode(Buzzer,OUTPUT);
    pinMode(FCoursS,INPUT);pinMode(13,OUTPUT);
    //### Serial.println("#### INITIALISATION DU SYSTEME ####...");
    //--------------------------------------------------------
    bip(800,300);bip(800,300);
    digitalWrite(13,HIGH);
    esp8266.begin(115200);
    sendCommand("AT",10,"OK");
    sendCommand("AT+CWMODE=1",10,"OK");
    sendCommand("AT+CWJAP=\""+ AP +"\",\""+ PASS +"\"",20,"OK");
    digitalWrite(13,LOW);
    while(!RESULTAT){
      bip(800,300);bip(800,300);
      digitalWrite(13,HIGH);
      bip(800,300);bip(800,300);
      digitalWrite(13,LOW);
    }
    bip(1200,1000);bip(1200,200);bip(1200,100);
    ServeurSave=millis();
    
    EtatFCT =0; EtatFCS =0;  
}
//###################################################################
void loop(){
    Scan();
    Traitement();
    ACTIONS();
}
//###################################################################
void Scan(){
    ReceptionSerie();
    LUMUNOSITE=Lumier();
    FLAM=DETECTEUR_FLAM();
    CapteurDHT11();
    MOVEMENT=DETECTEUR_MOVEMENT();
    if(digitalRead(FCoursT)==HIGH){ EtatFCT =1; } else { EtatFCT =0; }
    if(digitalRead(FCoursS)==HIGH){ EtatFCS =1; } else { EtatFCS =0; }
}
//###################################################################
void Traitement(){
   //###########################################
     //if(MOVEMENT==1){
        //CMP_Buzzer=10;
    //}
  //###########################################
}
//###################################################################
void ACTIONS(){
  if(MOVEMENT==1){
        CMP_Buzzer=2;
    }
  //---------------------------------------------------------
  //---------     envoi des donnees au Serveur WEB    -------
  if(millis()-ServeurSave>=DelaisEnvoiServeur){
    EnvoiData(TEMPERATURE01,HUMIDITE01,TEMPERATURE01+random(1,2),HUMIDITE01+random(-1,-7),LUMUNOSITE);
    ServeurSave=millis();
    DelaisEnvoiServeur=1800000; // MODIFIER CETTE VALEUR 10SC=10000, 10MN=600000,  
  }
  //---------------------------------------------------------
  //---------       envoi des alertes par SMS         -------
  if((millis()-SMS_FLAM_Save>=DelaisEnvoiSMS_FLAM)&&(FLAM<250)){
    CMP_Buzzer=2;
    SIM900_EMETEUR_MSG(MSG_FLAM);
    SMS_FLAM_Save=millis();
    DelaisEnvoiSMS_FLAM=100000;
  }

  if((millis()-SMS_cptT_Save>=DelaisEnvoiSMS_T) &&(EtatFCT==1)){
    CMP_Buzzer=2;
    SIM900_EMETEUR_MSG(MSG_VOLEUR1);
    SMS_cptT_Save=millis();
    DelaisEnvoiSMS_T=100000;
  }

  if((millis()-SMS_cptS_Save>=DelaisEnvoiSMS_S) &&(EtatFCS==1)){
    CMP_Buzzer=2;
    SIM900_EMETEUR_MSG(MSG_VOLEUR2);
    SMS_cptS_Save=millis();
    DelaisEnvoiSMS_S=100000;
  }
  if((millis()-SMS_TMP>=DelaisEnvoiSMS_TMP) &&(TEMPERATURE01>=55)){
    SIM900_EMETEUR_MSG(MSG_TMP);
    SMS_TMP=millis();
    DelaisEnvoiSMS_TMP=50000;
  }
  
  if((millis()-SMS_TMP1>=DelaisEnvoiSMS_TMP1) &&(TEMPERATURE01<=5)){
    SIM900_EMETEUR_MSG(MSG_TMP1);
    SMS_TMP1=millis();
    DelaisEnvoiSMS_TMP1=50000;
  }


   //--------------------------------------------------------------------
   //if((millis()-SMS_HUM_MAX>=DelaisEnvoiSMS_HUM_MAX) &&(HUMIDITE01>=HUM_MAX)){//###################################
    //SIM900_EMETEUR_MSG(MSG_HUM_MAX);
    //SMS_HUM_MAX=millis();
    //DelaisEnvoiSMS_HUM_MAX=50000;
 // }
  
  //if((millis()-SMS_HUM_MIN>=DelaisEnvoiSMS_HUM_MIN) &&(HUMIDITE01<=HUM_MIN)){//##################################
    //SIM900_EMETEUR_MSG(MSG_HUM_MIN);
   // SMS_HUM_MIN=millis();
    //DelaisEnvoiSMS_HUM_MIN=50000;
  //}
  
  //---------------------------------------------------------
  //---------     CAPTEUR DE MOUVEMENT     ------------------
   if(CMP_Buzzer>0){
      tone(Buzzer,800);
      CMP_Buzzer--;
   }else{
      noTone(Buzzer);
  }

   //--------------------------------------------------------------------
 //  if((millis()-SMS_MOUVEMENT>=DelaisEnvoiSMS_MOUVEMENT) &&(CMP_Buzzer==1)){//###################################
  //  SIM900_EMETEUR_MSG(MSG_MOUVEMENT);
   // SMS_MOUVEMENT=millis();
  //  DelaisEnvoiSMS_MOUVEMENT=50000;
 // }
  
  //---------------------------------------------------------
  //---------------------------------------------------------
  //-------------  COMMANDE MANUELS   -----------------------
  if (MSGC.length() >0){
    if(compar("AFF")){ affichage();}
    if(compar("ENV SERVEUR")){ ServeurSave=0;}


    MSGC="";
  }
  //---------------------------------------------------------

} 
//###################################################################
bool compar(String x){
int ok=0;
 for(int i=1; i<=x.length();i++){
 if(MSGC[i-1]!=x[i-1])ok=1;
 }
 if(ok == 1){
  return false;
  }else{
  return true;
  }
}
//###########################################################
void CapteurDHT11(){
  float hINT = dht01.readHumidity();
  float tINT = dht01.readTemperature();
  float hEXT = dht02.readHumidity();
  float tEXT = dht02.readTemperature();
  TEMPERATURE01=tINT;
  HUMIDITE01=hINT;
  TEMPERATURE02=tEXT;
  HUMIDITE02=hEXT;
  /*
  if (isnan(hINT) || isnan(tINT) ) {
    Serial.println("ARDUINO Erreur de lecture DHT INT !");
    return;
  } 
  if (isnan(hEXT) || isnan(tEXT) ) {
    Serial.println("ARDUINO Erreur de lecture DHT EXT !");
    return;
  } */
}
//###########################################################
int Lumier(){
 int valeur=analogRead(CapteurLUM);
 return valeur;
 }     
//###########################################################
int DETECTEUR_FLAM(){
 int valeur=analogRead(CapteurFLAM);
     if(valeur>=100){
     
     }
 return valeur;
 } 
//###########################################################
void affichage(){
  //TEMPERATURE=0,HUMIDITE=0,LUMUNOSITE=0,FLAM=0;
   Serial.print("HumInterieur: "); Serial.print(HUMIDITE01);Serial.print(" %\t");
   Serial.print("TempInterieur: "); Serial.print(TEMPERATURE01);Serial.println(" C");
   Serial.print("HumExterieur: "); Serial.print(HUMIDITE02);Serial.print(" %\t");
   Serial.print("TempExterieur: "); Serial.print(TEMPERATURE02);Serial.println(" C");
   Serial.print("LUM: "); Serial.print(LUMUNOSITE);Serial.print("\t");
   Serial.print("FLAM: "); Serial.println(FLAM);
   Serial.print("EtatFCT: "); Serial.print(EtatFCT);
   Serial.print("  EtatFCS: "); Serial.println(EtatFCS);
   Serial.print("  Mouvement: "); Serial.println(MOVEMENT);
}
//###########################################################
void EnvoiData(int TMPint,int HUMint,int TMPext,int HUMext,int LUM){
 //valSensor = getSensorData();
 String getData = "GET /update?api_key="+ API +
 "&"+ field1 +"="+String(TMPint)+
 "&"+ field2 +"="+String(TMPext)+
 "&"+ field3 +"="+String(HUMint)+
 "&"+ field4 +"="+String(HUMext)+
 "&"+ field5 +"="+String(LUM);
 sendCommand("AT+CIPMUX=1",10,"OK");
 sendCommand("AT+CIPSTART=0,\"TCP\",\""+ HOST +"\","+ PORT,15,"OK");
 sendCommand("AT+CIPSEND=0," +String(getData.length()+4),10,">");
 esp8266.println(getData);delay(1500);countTrueCommand++;
 sendCommand("AT+CIPCLOSE=0",10,"OK");
}
//###########################################################
int getSensorData(){
  return random(1000); // Replace with 
}
//###########################################################
void sendCommand(String command, int maxTime, char readReplay[]) {
  RESULTAT=false;
  //### Serial.print(countTrueCommand);
  //### Serial.print(". at command => ");
  //### Serial.print(command);
  //### Serial.print(" ");
  while(countTimeCommand < (maxTime*1))
  {
    esp8266.println(command);//at+cipsend
    if(esp8266.find(readReplay))//ok
    {
      found = true;
      break;
    }
  
    countTimeCommand++;
  }
  
  if(found == true)
  {
    //### Serial.println("BIEN...");
    RESULTAT=true;
    countTrueCommand++;
    countTimeCommand = 0;
  }
  
  if(found == false)
  {
    //### Serial.println("ERREUR");
    RESULTAT=false;
    countTrueCommand = 0;
    countTimeCommand = 0;
  }
  
  found = false;
 }
 //#########################################################################
 //#######################################################
void bip(int frq,int Tm){
 tone(Buzzer,frq); 
 delay(Tm);
 noTone(Buzzer);
 delay(Tm);
} 
//#######################################################
int DETECTEUR_MOVEMENT(){
 int valeur=digitalRead(CapteurMOV);
 return valeur;
} 
//#######################################################
//###################################################################
void ReceptionSerie(){
  while(Serial.available())
 {
  delay(5);
  char c = Serial.read();    
  MSGC = MSGC+c;
 }  
}
//###################################################################
void SIM900_EMETEUR_MSG(String MSG){
  delay(500);
  String DATA="AT+CMGS=\""+VOTRE_NUMERO+"\"\r";
  Serial.print(DATA);
  delay(1000);
  Serial.print(MSG);
  delay(1000);
  Serial.print(char(26));
  delay(1000);
} 
//###########################################################

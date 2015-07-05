/**
* Este código recoge un tempo (uint8_t) a través de Bluetooth. Después lo reenvía a los dispositivos 
* XBee que se hayan indicado (utilizando otro como emisor).
* Bibliotecas utilizadas:
* SoftwareSerial: https://www.arduino.cc/en/pmwiki.php?n=Reference/SoftwareSerial
* XBee: https://github.com/andrewrapp/xbee-arduino
* TimerOne: http://playground.arduino.cc/Code/Timer1
* @author Israel Blancas Álvarez
* Licencia: GPL V3
**/

/**
 * Para las pruebas se ha utilizado un Arduino Uno original
*/

#include <SoftwareSerial.h>//Permite emular por software un Serial, que usaremos para Bluetooth
#include <XBee.h> //Facilita el envío de datos usando XBee
#include "TimerOne.h" //Mejora el acceso al timer 1 de Arduino

SoftwareSerial BTDeviceHC(10, 11); //Creación del puerto serial por software para el Bluetooth
XBee xbee;//Creación del enlace con XBee

bool leer=true;//Leer o no de BT (así no hay que preguntar al serial si hay algo disponible -es más lento-)
int BTpin=7; //Pin que alimenta a BT


void setup() {
  BTDeviceHC.begin(9600);//Inicio del puerto serial del Bluetooth
  Serial.begin(9600);//Incio del puerto serial XBee
  
  while (!Serial){}
    
  xbee = XBee();
  xbee.setSerial(Serial);//Asociación de serial al XBee
  
  
  BTDeviceHC.println("Iniciando...");
  
  
  pinMode(BTpin,OUTPUT);//Se establece como salida el pin que alimenta al BT
  digitalWrite(BTpin, HIGH);//Se pone en estado alto
}




uint8_t payload[] = { 0x7F,'<',0x7F };//Payload a enviar por XBee

XBeeAddress64 primera = XBeeAddress64(0x0013A200, 0x4079E3ED);//Dirección XBee A
XBeeAddress64 segunda = XBeeAddress64(0x0013A200, 0x40A09C69);//Dirección XBee B

uint16_t addr16=0xffff;//Dirección 16 bit PAN
uint8_t broadcastradius=0;//Radio de broadcast
uint8_t option=0;//Opciones de comprobación
uint8_t frameid=0;//ID del marco (un ID mayor activa comprobaciones)

ZBTxRequest primeraS=ZBTxRequest(primera,addr16,broadcastradius,option,payload,sizeof(payload),frameid);//Creación del paquete A
ZBTxRequest segundaS=ZBTxRequest(segunda,addr16,broadcastradius,option,payload,sizeof(payload),frameid);//Creación del paquete B


long tiempo=0;//Última vez que se midió el tiempo

uint8_t tempo='<';//Tempo guardado
long periodo=1000;

bool turno=true;

/**
* Función llamada por el timer. Envía los datos a los XBee si ha pasado el suficiente tiempo desde el último envío
*/
void Envio(void){
    if(millis()-tiempo>periodo){
      if(turno)
       xbee.send(primeraS);
       
      else
        xbee.send(segundaS);
      turno=!turno; //Una vez se sincroniza uno y otra vez otro
      tiempo=millis();
     }
}



void loop() {
    if(leer){//Si activado
         if (BTDeviceHC.available()){

           leer=false;//Desactivamos nuevas lecturas
           
           tempo = BTDeviceHC.read();//Leemos tempo (Pulsos por minuto)
           
           float au1= (float)tempo/(float)60;// Calculamos pulsos por segundo
           float au2 = (float)1000 / (float)au1;// Calculamos tiempo entre pulsos
           periodo = (long) au2*4; // Es el cuádruple (en el receptor se harán las subdivisiones)
           
           //Se regeneran los paquetes
           payload[1]=tempo;
           primeraS=ZBTxRequest(primera,addr16,broadcastradius,option,payload,sizeof(payload),frameid);
           segundaS=ZBTxRequest(segunda,addr16,broadcastradius,option,payload,sizeof(payload),frameid);
           
           
           
           digitalWrite(BTpin, LOW); // Apagamos el dipositivo BT
           Timer1.initialize(500);  // Iniciamos timer
        }
    }
}

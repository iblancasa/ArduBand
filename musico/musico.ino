/**
* Este código recoge un tempo (uint8_t) a través de XBee. Se ajustan los tiempos y hace funcionar un actuador
* manteniendo un pulso
* Bibliotecas utilizadas:
* XBee: https://github.com/andrewrapp/xbee-arduino
* TimerOne: http://playground.arduino.cc/Code/Timer1
* @author Israel Blancas Álvarez
* Licencia: GPLv3
**/

/**
 * La implementación final se ha hecho con un derivado de 
 * Arduino Lilypad. El IDE Arduino piensa que la velocidad
 * de reloj de esta placa es de 8Mhz cuando, realmente
 * funciona a 16Mhz. Es por eso que se multiplican los tiempos 
 * por 2 (para que tarde el doble de tiempo y sea el tiempo que realmente
 * debe esperar). Por ejemplo:
 * Si queremos que algo se haga cada 1 segundo, nuestra placa
 * lo hará cada 500ms. Multiplicando por dos, se soluciona el problema
 * Lo mismo pasa con el puerto serie: abrirlo a 4800bps, significa que, realmente,
 * estará funcionando a 9600 bps
*/


#include <XBee.h> //Facilita el envío de datos usando XBee
#include <TimerOne.h> //Mejora el acceso al timer 1 de Arduino


XBee xbee;//Creación del enlace con XBee
const int motor = 2;//Pin del actuador


void setup() {
  xbee = XBee();
  Serial.begin(4800);//Incio del puerto serial XBee
  xbee.setSerial(Serial);//Asignación del puerto serial
  
  pinMode(motor, OUTPUT);//Salida del actuador
  
  Timer1.initialize(500);//Inicialización del timer
  Timer1.attachInterrupt(vibrar);//Asignación de la función
  
}


bool entrar = false;//Variable auxiliar para parseo de trama
long frecuencia = 2000;//Cada cuanto marcar un pulso
int recibido;//Caracter recibido
long antes=millis();//Última vez que se midió el tiempo


/**
* Función llamada por el timer. Se encarga de activar o desactivar el actuador en función del tiempo que haya pasado
*/
void vibrar(void){
    if(millis() - antes > frecuencia ){//Tiempo para volver a activar
        digitalWrite(motor, HIGH);
        antes = millis();
    }
    
    else if(millis() - antes > 300){//Tiempo para desactivar
      digitalWrite(motor, LOW);
    } 
}



void loop() {
   if(Serial.available()>=21){//Si han llegado datos de XBee
     //Parseo de la trama
     while(Serial.available()>0){
        recibido = Serial.read();
         if(entrar){
              float au1= (float)recibido/(float)60;
              float au2 = (float)1000 / (float)au1;
              frecuencia = (long) au2*2;  
              Serial.read();//Descartar el último símbolo
              entrar = false;//Fin del parseo
              digitalWrite(motor, HIGH);//Activar actuador
              antes = millis();//Tomar tiempo
          }
          else if(recibido == 0x7F ){//Si está el delimitador
            entrar=true;
          }
    }
  }
}

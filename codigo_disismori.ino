#include <SoftwareSerial.h>
SoftwareSerial SIM900(7, 8);
 #include <Wire.h> // LibrerÃ­a comunicaciÃ³n I2C
  #include "DHT.h"
  #define DHTPIN 4     // Pin donde estÃ¡ conectado el sensor
  #define DHTTYPE DHT22   // Sensor DHT22
//Vcc=5V arduino
//gnd =gnd arduino
  DHT dht(DHTPIN, DHTTYPE);
  const int sensor = 2; // Pin digital para el sensor de flujo YF-S201
  int litros_Hora=0; // Variable que almacena el caudal (L/hora)
  volatile int pulsos = 0; // Variable que almacena el nÃºmero de pulsos
  unsigned long tiempoAnterior = 0; // Variable para calcular el tiempo transcurrido
  unsigned long pulsos_Acumulados = 0; // Variable que almacena el nÃºmero de pulsos acumulados
  float litros; // // Variable que almacena el nÃºmero de litros acumulados
  const int EchoPin = 5; //Se conecta Echo pin digital 5
  const int TriggerPin = 6;// Seconecta trigger pin digital 6
 
  // Rutina de servicio de la interrupciÃ³n (ISR)
      void flujo(){
            pulsos++; // Incrementa en una unidad el nÃºmero de pulsos
      }
void setup() {
 digitalWrite(9, HIGH); // Descomentar para activar la alimentación de la tarjeta por Software
delay(1000); 
digitalWrite(9, LOW);
delay(5000);
  // put your setup code here, to run once:
SIM900.begin(19200); //Configura velocidad del puerto serie para el SIM900
  Serial.begin(19200); //Configura velocidad del puerto serie del Arduino
  delay(1000);
  comandosAT();
   Serial.println("Iniciando...");
        dht.begin();
        //YF-S201 SENSOR DE CAUDAL
        pinMode(sensor, INPUT_PULLUP); // Pin digital como entrada con conexiÃ³n PULL-UP interna
        interrupts(); // Habilito las interrupciones
        // InterrupciÃ³n INT0, llama a la ISR llamada "flujo" en cada flanco de subida en el pin digital 2
        attachInterrupt(digitalPinToInterrupt(sensor), flujo, RISING);  
        tiempoAnterior = millis(); // Guardo el tiempo que tarda el ejecutarse el setup
        //SENSOR ULTRASONIDO
        pinMode(TriggerPin, OUTPUT);
        pinMode(EchoPin, INPUT);
}

void loop() {
   Serial.print(" Sensor Caudal\n");
              
               int litros=SENSORCAUDAL();
               delay(1000);
               Serial.print(" Sensor Humedad y temperatura\n");
               float temp=SENSORDHT();
               delay(1000);
               Serial.print(" Sensor Ultrasonido\n");
               float cm = ping(TriggerPin, EchoPin);
               Serial.print("Distancia: ");
               Serial.println(cm);
               Serial.println(String(cm)+" y "+String(litros)+" y "+String(temp));
               delay(1000);
  // put your main code here, to run repeatedly:
  http(String(cm),String(litros),String(temp));

}
void http(String cm,String l,String t){  
//String g="p="+cm+"&c="+l+"&t="+t+"&tu=5";
String g="p=10&c=10&t=10&tu=5";
SIM900.println("AT+HTTPPARA=\"URL\",\"http://disismori.herokuapp.com/index.php/App/insertar?"+g+"&a=1&po=2\"");
delay(1000);
mostrarDatos();
SIM900.println("AT+HTTPACTION=0");
delay(1000);
mostrarDatos();
SIM900.println("AT+HTTPREAD");
delay(1000);
mostrarDatos();
delay(20000);

}
void comandosAT(){
SIM900.println("AT+CIPSTATUS");//Consultar el estado actual de la conexiÃ³n
delay(1000);
SIM900.println("AT+CIPMUX=0");//comando configura el dispositivo para una conexiÃ³n IP Ãºnica o mÃºltiple 0=Ãºnica
delay(1000);
mostrarDatos();
//SIM900.println("AT+CSTT=\"internet.comcel.com.co\",\"COMCELWEB\",\"COMCELWEB\"");
//delay(1000);
//mostrarDatos();
delay(1000);
mostrarDatos();
SIM900.println("AT+CIICR");//REALIZAR UNA CONEXIÃ“N INALÃ�MBRICA CON GPRS O CSD
delay(1000);
mostrarDatos();
SIM900.println("AT+CIFSR");// Obtenemos nuestra IP local
delay(2000);
mostrarDatos();
SIM900.println("AT+CIPSPRT=0");//Establece un indicador '>' al enviar datos
delay(1000);
mostrarDatos();
SIM900.println("AT+CPIN?");
delay(1000);
mostrarDatos();
SIM900.println("AT+CGREG?");
delay(1000);
mostrarDatos();
SIM900.println("AT+COPS?");
delay(1000);
mostrarDatos();
SIM900.println("AT+CSQ");
delay(3000);
mostrarDatos();
SIM900.println("AT+SAPBR=3,1,\"Contype\",\"GPRS\"");
delay(1000);
mostrarDatos();
SIM900.println("AT+SAPBR=3,1,\"APN\",\"internet.comcel.com.co\"");
delay(1000);
mostrarDatos();
SIM900.println("AT+SAPBR=1,1");
delay(1000);
mostrarDatos();
SIM900.println("AT+SAPBR=2,1");
delay(1000);
mostrarDatos();
SIM900.println("AT+CGATT=1");
delay(1000);
mostrarDatos();
SIM900.println("AT+CGDCONT=1,\"IP\",\"internet.comcel.com.co\"");
delay(1000);
mostrarDatos();
SIM900.println("AT+CGDCONT=2,\"IP\",\"isp.cingular\"");
delay(1000);
mostrarDatos();
SIM900.println("at+cgdcont?");
delay(1000);
mostrarDatos();
SIM900.println("AT+CGACT=1,1");
delay(1000);
mostrarDatos();
SIM900.println("AT+HTTPINIT");
delay(1000);
mostrarDatos();
SIM900.println("AT+HTTPPARA=\"CID\",1");
delay(1000);
mostrarDatos();


}
 int SENSORCAUDAL(){
              // Cada segundo calculamos e imprimimos el caudal y el nÃºmero de litros consumidos
              if(millis() - tiempoAnterior > 1000)
              {
                // Realizo los cÃ¡lculos
                tiempoAnterior = millis(); // Actualizo el nuevo tiempo
                pulsos_Acumulados += pulsos; // NÃºmero de pulsos acumulados
                litros_Hora = (pulsos * 60 / 7.5); // Q = frecuencia * 60/ 7.5 (L/Hora)
                litros = pulsos_Acumulados*1.0/450; // Cada 450 pulsos son un litro
                pulsos = 0; // Pongo nuevamente el nÃºmero de pulsos a cero
               
               Serial.print(litros_Hora);
              Serial.print(" L/Hora      ");
              // Imprimo el nÃºmero de litros acumulados
             
              Serial.print(litros);    
              Serial.print(" Litros\n");
              
                return litros_Hora;
          }
        }
            float SENSORDHT() {
            delay(2000);
            float h = dht.readHumidity(); //Leemos la Humedad
            float t = dht.readTemperature(); //Leemos la temperatura en grados Celsius
            float f = dht.readTemperature(true); //Leemos la temperatura en grados Fahrenheit
            Serial.print("Humedad ");
            Serial.print(h);
            Serial.print(" %t");
            Serial.print("Temperatura: ");
            Serial.print(t);
            Serial.print(" *C ");
            Serial.print(f);
            Serial.println(" *F");

            return t;
      }
 
float ping(int TriggerPin, int EchoPin) {
   long duration, distanceCm;
   digitalWrite(TriggerPin, LOW);  //para generar un pulso limpio ponemos a LOW 4us
   delayMicroseconds(4);
   digitalWrite(TriggerPin, HIGH);  //generamos Trigger (disparo) de 10us
   delayMicroseconds(10);
   digitalWrite(TriggerPin, LOW);

   duration = pulseIn(EchoPin, HIGH);  //medimos el tiempo entre pulsos, en microsegundos
   
   distanceCm = duration * 10 / 292/ 2;   //convertimos a distancia, en cm
   double distanceM=distanceCm/100.0;
   return distanceM;
}
void mostrarDatos()//Muestra los datos que va entregando el sim900
{
while(SIM900.available()!=0)
Serial.write(SIM900.read());

}

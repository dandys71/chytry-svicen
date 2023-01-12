#include <Servo.h>
#include <avr/sleep.h>

Servo myservo;  // create servo object to control a servo
// twelve servo objects can be created on most boards
#define pohCidlo 2 //musí být v 2 nebo v 3
#define zvukCidlo 3
boolean val =0;

float pos = 0;    // variable to store the servo position
#define R  13
#define Y  12
#define G  11
#define S 10
const int T = A0;	/* LM35 O/P pin */
#define DELAY 80
const int OK_STATE_DELAY_SEC = 10;
int WARNING_STATE_DELAY_SEC = 10;
bool detekceZvukOn = true;
bool detekcePohybOn = true;
bool successCheck = false;
int detekceSec;
bool wasError = false;
int errorSec;
float degree = -1;
bool mode1 = true;

void setup() {
  Serial.begin(9600);

  WARNING_STATE_DELAY_SEC = OK_STATE_DELAY_SEC + WARNING_STATE_DELAY_SEC;
  
  pinMode(R, OUTPUT); //R
  pinMode(Y, OUTPUT); //Y
  pinMode(G, OUTPUT); //B
  pinMode(S, OUTPUT); //Bzucak


  rozsvit();
  myservo.attach(8);
  delay(500);
  myservo.detach();
  
  pinMode(zvukCidlo, INPUT);
  pinMode(pohCidlo, INPUT);

// při rostoucí hraně (logO->log1) se vykoná program prerus
  if(detekcePohybOn){
    attachInterrupt(0, detekcePohybu, RISING); //0 = pin 2
  }
  if(detekceZvukOn){
    attachInterrupt(1, detekceZvuku, RISING); // 1 = pin 3
  }
  Serial.println("START");
}

void loop() {
  if(mode1){
    if(!wasError){
      int diffSec = millis() / 1000 - detekceSec ;

      if(diffSec <= OK_STATE_DELAY_SEC){
        ok();
      }else if(diffSec <= WARNING_STATE_DELAY_SEC){
        warning();
      }
      else{ //pokud je error bude zbývat jen pář minut do vypnutí (1 minuta - nyní 5s - aby měl uživatel možnost proces vypínání přerusit hlukem nebo pohybem)
          error();
          zhasni();
          wasError = true;
          detachInterrupt(digitalPinToInterrupt(2));
          detachInterrupt(digitalPinToInterrupt(3));
          mode1 = false;
      }
    }
  }else{ //mode 2
  if(successCheck){
      float tempBefore = temp(); //měření teploty

      Serial.print("Temp Before = ");
      Serial.print(tempBefore);
      Serial.println(" Degree Celsius\n");

      delay(15 * 1000);
      float tempAfter = temp();

      Serial.print("Temp After = ");
      Serial.print(tempAfter);
      Serial.println(" Degree Celsius\n");

      if(tempAfter > (tempBefore)){
        alarm(3);
      }else{
        end();
      }
      Serial.println(tempBefore - tempAfter);
    }else{
      end();
    }
  }
}

void error(){
  digitalWrite(R, HIGH);
  digitalWrite(Y,LOW);
  digitalWrite(G, LOW); 
}

void warning(){
  digitalWrite(R, LOW);
  digitalWrite(Y,HIGH);
  digitalWrite(G, LOW); 
}

void ok(){
  digitalWrite(R, LOW);
  digitalWrite(Y,LOW);
  digitalWrite(G, HIGH); 
}

void led_off(){
  digitalWrite(R, LOW);
  digitalWrite(Y,LOW);
  digitalWrite(G, LOW); 
}

void end(){
  Serial.println("KONEC - USPECH");
  delay(1000);
  sleep();
}

void detekcePohybu() {
// pokud je aktivován digitální vstup,
// vypiš informaci po sériové lince
  Serial.println("Detekce pohybu!");
  detekceSec = millis() / 1000;
}


void detekceZvuku(){
  
  detekceSec = millis() / 1000;
  Serial.print(detekceSec);
  Serial.println("Detekce zvuku!");
}

void zhasni(){
  myservo.attach(8);
  int apos = myservo.read();
  for (pos = apos; pos <= (apos + 40); pos += 1) { 
    myservo.write(pos);             
    //delay(5);                    
  }
  delay(500);
  myservo.detach();
}

void rozsvit(){
  int apos = myservo.read();
for (pos = apos; pos >= (apos - 35); pos -= 1) { 
    myservo.write(pos);              
    delay(15);                      
  }
}

void alarm(int seconds){
  digitalWrite(S, HIGH);
  delay(seconds * 1000);
  digitalWrite(S, LOW);
}

float temp(){
  int temp_adc_val;
  float temp_val;

  float average_temp = 0;
  int temp_count = 10;

  for (int i = 0; i < temp_count; i++){
  temp_adc_val = analogRead(T);	/* Read Temperature */
  temp_val = (temp_adc_val * 4.8828125);	/* Convert adc value to equivalent voltage */
  temp_val = ((temp_val - 25) / 10.0);	/* LM35 gives output of 10mv/°C */
  

  average_temp += temp_val;  
 
  delay(100);
  }
  
  average_temp = average_temp / temp_count;

  return average_temp;
}

void sleep(){
  sleep_enable();
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  led_off();
  delay(1000);
  sleep_cpu();
}
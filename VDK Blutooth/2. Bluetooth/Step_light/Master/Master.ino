
// Master
#include <SoftwareSerial.h>
SoftwareSerial BTSerial(10,11); // RX, TX cua cai board
// sensor
int value;
//
void setup() {
  BTSerial.begin(38400);
  Serial.begin(9600);
}

void loop() {
  value = analogRead(A0);
  Serial.println(value);
  if (value <= 650) {
    BTSerial.write('0');
    Serial.println("gui du lieu 0 ");
  } else {
    BTSerial.write('1');
     Serial.println("gui du lieu 1 ");
  }
  delay(1500);
}


//slave



#include <i2cMultiSm.h>

SVMULTI vcs;
void setup() {
   Serial.begin(115200);
  vcs.init(101);
}

void loop() {
  vcs.newReading(); // start sensor reading, after swithc on it iss done auto
  delay(100); //let sensor read data
  soil_ret_t multi;
  vcs.getData(&multi);
  Serial.println("-----");
    Serial.print("s1");
    Serial.print("=");
    Serial.println(multi.vwc[0]);
    Serial.print("s2");
    Serial.print("=");
    Serial.println(multi.vwc[1]);
    Serial.print("s3");
    Serial.print("=");
    Serial.println(multi.vwc[2]);
    Serial.print("s4");
    Serial.print("=");
    Serial.println(multi.vwc[3]);
    Serial.print("s5");
    Serial.print("=");
    Serial.println(multi.vwc[4]);
    Serial.print("t1");
    Serial.print("=");
    Serial.println(multi.temp[0]);
    Serial.print("t2");
    Serial.print("=");
    Serial.println(multi.temp[1]);
  delay(1000);
} 

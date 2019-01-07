#include <PulsePosition.h>

#define RC_IN 5
#define RC_TRANSMIT_TIME 10
#define LOOP_TIME 10

PulsePositionInput rcInput;

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("Hello Teensy");

  rcInput.begin(RC_IN);

  delay(3000);
}

float fconstrain(float v, float nmin, float nmax) {
  return max(min(nmax, v), nmin);
}

void loop() {
  static unsigned long rcPiOutTime = 0L;

  // Serial.println("loop");
  
  unsigned long timeStamp = millis();
  int count = rcInput.available();
  
  // Serial.print("Count: "); Serial.println(count);
  if (count > 0) {
    if (timeStamp - rcPiOutTime >= RC_TRANSMIT_TIME) {
      Serial.print("I ");
      Serial.print(rcInput.read(1), 1); Serial.print("  "); Serial.print(rcInput.read(2), 1); Serial.print("  ");
      Serial.print(rcInput.read(3), 1); Serial.print("  "); Serial.print(rcInput.read(4), 1); Serial.print("  ");
      Serial.print(rcInput.read(5), 1); Serial.print("  "); Serial.print(rcInput.read(6), 1); Serial.print("  ");
      Serial.print(rcInput.read(7), 1); Serial.print("  "); Serial.println(rcInput.read(8), 1);

      rcPiOutTime = timeStamp;
    }
    else {
      // Serial.println("No Time yet");
    }
  }
  else {
    // Serial.println("No available");
  }

  delay(25);
}

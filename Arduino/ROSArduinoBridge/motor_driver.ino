#include <SparkFun_TB6612.h>
#include "motor_driver.h"

Motor *motors[NUM_MOTORS];
Motor motor0 = Motor(12, 11, 10, 1, 35);
Motor motor1 = Motor(34, 33, 9, 1, 35);
Motor motor2 = Motor(18, 19, 23, 1, 17);
Motor motor3 = Motor(16, 15, 22, 1, 17);
Motor motor4 = Motor(39, 14, 21, 1, 38);
Motor motor5 = Motor(37, 36, 20, 1, 38);

void initMotorController() {
  motors[0] = &motor0;
  motors[1] = &motor1;
  motors[2] = &motor2;
  motors[3] = &motor3;
  motors[4] = &motor4;
  motors[5] = &motor5;
}

void setMotorSpeed(int i, int spd) {
  if (i < 0 || i >= NUM_MOTORS) {
    return;
  }

  if (spd < -255) {
    spd = -255;
  }

  if (spd > 255) {
    spd = 255;
  }
  // Serial.print("motor: "); Serial.print(i); Serial.print(" --> "); Serial.println(spd);
  motors[i]->drive(spd);
}

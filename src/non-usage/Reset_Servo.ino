#include <Servo.h>

#define DEF_DELAY 500

Servo myServo[4][3];
const int servo_pin[4][3] = { {2, 3, 4}, {5, 6, 7}, {8, 9, 10}, {11, 12, 13} };


void setup() {

    //serial connection

    Serial.begin(9600);
    //attach servo

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 3; j++) {
            myServo[i][j].attach(servo_pin[i][j]);
            delay(DEF_DELAY);

            Serial.println(myServo[i][j].read());
        }
    }


    Serial.println("\n --------------------------------- \n");
    delay(DEF_DELAY);

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 3; j++) {
            myServo[i][j].write(90); //90도로 리셋

            delay(DEF_DELAY);

            Serial.println(myServo[i][j].read());
        }
    }
}

void loop() {
    //void
}


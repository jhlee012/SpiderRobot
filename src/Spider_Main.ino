#include <main_led.h>
#include <Adafruit_NeoPixel.h>


LEDControl ledMain;

void setup() {
    Serial.begin(9800);

    FrontPixels.begin();
    BackPixels.begin();

    FrontPixels.clear();
    BackPixels.clear();
    delay(1000);

    ledMain.All_PixelChange(10, 150, 100, true);

    Serial.println("Done");
}

void loop() {
    //empty
}

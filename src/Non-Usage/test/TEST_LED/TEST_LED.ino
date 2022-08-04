#include <main_led.h>
#include <Adafruit_NeoPixel.h>

LEDControl ledMain;

void setup() {;
    ledMain.begin();
    delay(500);

    ledMain.clear();
    
    ledMain.All_PixelChange(0, 255, 0, true);
}

void loop() {
    //void
}

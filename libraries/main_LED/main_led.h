#include <Adafruit_NeoPixel.h>

#define PIXCOUNT 12
#define F_PIN 17
#define B_PIN 18


Adafruit_NeoPixel FrontPixels(PIXCOUNT, F_PIN, NEO_GRB + NEO_KHZ800); //앞쪽 네오픽셀 "FrontPixels"
Adafruit_NeoPixel BackPixels(PIXCOUNT, B_PIN, NEO_GRB + NEO_KHZ800); // 뒤쪽 네오픽셀 "BackPixels"

/* #define NUMPIX 12 //네오픽셀 LED의 픽셀 수 (상수)
#define F_PIN 17 // 앞쪽 LED의 아두이노 핀 (변수)
#define B_PIN 18 // 뒤쪽 LED의 아두이노 핀 (변수) */


/*! **

Neopixel Library; Main class public1 constructurs:

.setPixelColor(int pixelnumber, Constructor())  => pixelnumber 째 픽셀의 컬러를 Constructor 반환값만큼 설정 // @return void
.Color(int r, int g, int b) => SetPixelColor() 에서 주로 사용 (uint8 rgb값 리턴) // @return uint8 (rgbs)
.show() //.setPixelColor() 로 변경된 값 적용 // @return void

** .setPixelColor() 로 각 픽셀의 컬러 할당 시 최소 300ms의 대기 시간 필요**

*/

/**
* @file main_led.h
* @author Dev.Jh / Contributed by Adafruit, Neopixel and other Group-5 teammates
* @copyright Copyright 2022. DevJh All rights reserved.
*
* @brief 
* This Library was developed for Sunmun_Univ Gaon_hs collaboration summer school project. Developed by Dev.Jh, based on Adafruit's Neopixel Library and hardwares.
* Do not use this as any other way except for test.
*
* @todo 
* Paragraph descriptions ; Parameter Descriptions ; as well
*/




class LEDControl {
    public :

        /** @param show :: boolean parameters all mean "Call .show() function or not" */

        void All_PixelChange(int r, int g, int b, bool show) //모든 픽셀의 컬러 변경 ; 매개변수 r, g, b이용
        
        { 
            for (int i = 0; i <= PIXCOUNT; i++) {
            FrontPixels.setPixelColor(i, FrontPixels.Color(r, g, b));
            BackPixels.setPixelColor(i, BackPixels.Color(r,g,b));
        }

            if (show = true) {
                FrontPixels.show();
                BackPixels.show();
            }

        }

        /**
        @brief 1. Change Color for all pixels (Value of NUMPIX)
        2. Serial Print "All Color changed"

        @return void, return Serial.println() function
        */

        void F_PixelChange(int r, int g, int b, bool show) { //앞쪽 픽셀의 컬러 모두 변경 ; 매개변수 All_과 동일
            for (int i = 0; i <= PIXCOUNT; i++) {
            FrontPixels.setPixelColor(i, FrontPixels.Color(r, g, b));
            }

            if (show = true) 
                FrontPixels.show();
        }

        //! @brief Same with All_PixelChange()
        //! @return void, return Serial.println() function

        void B_PixelChange(int r, int g, int b, bool show) { //뒤쪽 픽셀의 컬러 모두 변경 ; 매개변수 동일
            for (int i = 0; i <= PIXCOUNT; i++) {
            BackPixels.setPixelColor(i, BackPixels.Color(r,g,b));
        }
            if (show = true) 
                BackPixels.show();
        }

        /**
        @brief Same with F_PixelChange()
        @return void, Serial Function()
        */

/*

        void S_PixelChange(int num, int r, int g, int b, bool show) {
            Adafruit_NeoPixel temp;
            int tempnum = 0;
            if (0 < num <= 12) {
                temp  = FrontPixels;
                tempnum = num;
            }
                
            else if (12 < num <= 24) {
                temp = BackPixels;
                tempnum = num - 12;               
            } 
            
            temp.setPixelColor(tempnum, temp.Color(r, g, b));

            if (show = true) 
                temp.show();
        }
*/

        /**
        @param 
        int num :  Number of NeoPixel's pixel | 1 - 12 ; Front | 13 - 24 ; Back
        int r, g, b : RGB Color UINT8 value
        bool show : if true => call function .show() ? False
        @brief
        Specific Pixel Change - Currently not support RANGE change ; comming soon
        @return
        typeof void | Serial.println function()
        */
};

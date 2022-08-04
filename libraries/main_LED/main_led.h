#include <Adafruit_NeoPixel.h>
#include <Servo.h>

#define PIXCOUNT 12 /** @def 픽셀 수 (NeoPixel) */
#define F_PIN 17 /** @def 앞쪽 LED 연결핀 ; 아날로그 변환됨 */
#define B_PIN 18 /** @def 뒷쪽 LED 연결핀 ; 아날로그 변환됨 */


Adafruit_NeoPixel FrontPixels(PIXCOUNT, F_PIN, NEO_GRB + NEO_KHZ800); /** 앞쪽 네오픽셀 "FrontPixels" */
Adafruit_NeoPixel BackPixels(PIXCOUNT, B_PIN, NEO_GRB + NEO_KHZ800); // 뒤쪽 네오픽셀 "BackPixels"

/**
 * @file main_led.h
 * @author Dev.Jh / Adafruit Library by Adafruit.co / Contributions::roup-5 teammates
 * @copyright Copyright 2022. DevJh All rights reserved.
 *
 * @brief 
 * This Library was developed for SNU & Gaon summer school project. Developed by Dev.Jh, based on Adafruit's Neopixel Library and hardwares.
 * Do not use this as any other way except for test.
 * 
 * @date Last Edited at ; 2022/08/03
 *
 * @details PROJECT NOT ENDED YET -- UNTIL AGUST 5, 2022 [WIP]
 *
 * @todo 
 * Paragraph descriptions ; Parameter Descriptions ; as well
 * Respective LED control
 *
 * @warning .setPixelColor() 로 각 픽셀의 컬러 할당 시 최소 300ms의 대기 시간 필요
*/





//LED Control Function
/**
 * @brief LEDControl name; 으로 선언 시 이미 헤더 인클루드로 정의된 NeoPixel 객체를 활용하여 .begin() 및 .clear() 실행
 * 
 * @paragraph LedControl-Class-Declare LED Control
 *        
 * 
 */
class LEDControl {
    public :
        LEDControl() {

        };

        /**
         * @brief Get the brightness object of each lED 
         * 
         * @param decider 0 => front, 1 => back (Invalid Decider => Return NULL)
         * @return int 
         */

        void begin() {
            FrontPixels.begin();
            BackPixels.begin();
        }

        void clear() {
            FrontPixels.clear();
            BackPixels.clear();
        }

        int get_brightness(int decider) {
            switch(decider) {
                case 0:
                    return FrontPixels.getBrightness();
                    break;
                case 1: 
                    return BackPixels.getBrightness();
                    break;
                default:
                    return 0; //invalid parameter ; decider
            }      
        }

        /**
         * @brief Check Each LED's brightness correspondence
         * 
         * @note I dont know if "bool" type is available to return null; so default switch returns "false"
         * 
         * @param decider 0 => all ,1 => front, 2 => back
         * @param value compare with
         * @param change if true, change with this , yeah
         * @return true 
         * @return false 
         */

        bool check_brightness(int decider, int value) {
            switch(decider) {
                case 0:
                    if (FrontPixels.getBrightness() == value && BackPixels.getBrightness() == value)
                        return true;
                    else return false;
                    break;
                case 1:
                    if (FrontPixels.getBrightness() == value)
                        return true;
                    else return false;
                    break;
                case 2:
                    if (BackPixels.getBrightness() == value)
                        return true;
                    else return false;
                    break;
                default:
                    Serial.println("Invalid Parameter : decider");
                    return false;
            }
        }

        /** ALL: @param show :: boolean parameters all mean "Call .show() function or not" */

        /** brightness
         * @brief Change Pixel's birghtness (can decide with decider)
         * 
         * @param decider 0 => all , 1 => front, 2 => back
         * @param value 0~255 brightness
         */

        void brightness(int decider, int value) {
            if (value > 255) return;
            if (value < 0 ) return;
            if (decider == 0) {
                FrontPixels.setBrightness(value);
                BackPixels.setBrightness(value);
            }
            else if (decider == 1) {
                FrontPixels.setBrightness(value);
            } else if (decider == 2) {
                BackPixels.setBrightness(value);
            }
        }

        /**
         * @brief Change All Pixel's Color, and decide show or not
         * 
         * @param r => uint8 red value
         * @param g => uint8 green value
         * @param b => uint8 blue value
         * @param show :: boolean parameters all mean "Call .show() function or not"
         */
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

        void F_PixelChange(int r, int g, int b, bool show) { //앞쪽 픽셀의 컬러 모두 변경 ; 매개변수 All_과 동일
            for (int i = 0; i <= PIXCOUNT; i++) {
            FrontPixels.setPixelColor(i, FrontPixels.Color(r, g, b));
            }

            if (show = true) 
                FrontPixels.show();
        }

        void B_PixelChange(int r, int g, int b, bool show) { //뒤쪽 픽셀의 컬러 모두 변경 ; 매개변수 동일
            for (int i = 0; i <= PIXCOUNT; i++) {
            BackPixels.setPixelColor(i, BackPixels.Color(r,g,b));
        }
            if (show = true) 
                BackPixels.show();
        }

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

        /**
         * @brief RGB Cycle Command
         * 
         * @param decider ; Target Detection | Only "0" or "1" | 0 => FrontPixels | 1 -> BackPixels
         * @param count ; Cycle count to run | etc) 1 => total 12 pixels , 2 => total 24 pixels
         */


       void WaveRGB(int decider, int count) {
/*             Adafruit_NeoPixel temp;

            if (decider == 0) {
                temp = FrontPixels;
            } else {
                temp = BackPixels;
            }

            int r = 0;
            int g = 255;
            int b = 0;

            int toggle = 0;


            int temp2; 
            for (int i = 0; i < count*12; i++) {
                temp2 = i;
                while(temp2>12) {
                    temp2 -= 12;
                }

                if (toggle == 0) {
                    r += temp2;
                    g -= temp2;
                    b += temp2;
                } else if (toggle == 1) {
                    r -= temp2;
                    g += temp2;
                    b -= temp2;
                }

                if (r < 0 || g < 0 || b < 0) {
                    toggle = (toggle == 0) ? 1 : 0;
                } else if (r > 255 || g > 255 || b > 255) {
                    toggle = (toggle == 0) ? 1 : 0;

                    r -= 255;
                    g -= 255;
                    b -= 255;
                    
                }

                r = abs(r);
                g = abs(g);
                b = abs(b);
                

                temp.setPixelColor(temp2, r, g, b);
            } */

            Adafruit_NeoPixel temp;

            if (decider == 0) {
                temp = FrontPixels;
            } else {
                temp = BackPixels;
            }

            for (int i = 0; i <count*12; i++) {
                int num = i;
                while(num > 12) {
                    num = num - 12;
                }            

                temp.setPixelColor(num, 255/num, abs(255-num), abs(num*3-255));
                delay(300);
                temp.show();
            }


       }


        /**
         * @brief 
         * 
         * @param decider => 0 => FrontPixels, 1 => BackPixels
         * @param a, b => Select Range [a, b] and set these pixels
         * @param show => boolean ".show()" usage
         */
       void Range_PixelChange(int decider, int a, int b, int r, int g, int bl, bool show) {
            //sorry for too many params ;;

            Adafruit_NeoPixel temp; 

            switch(decider) {
                case 0:
                    temp = FrontPixels;
                    break;
                case 1: 
                    temp = BackPixels;
                    break;
                default:
                    return;
            }

            if (a <= 0 || b <= 0  || a >12 || b> 12 || a > b) 
                return;
            
            if (r > 255 || g > 255 || bl > 255 || r < 0 || g < 0 || bl < 0)
            
            while (a <= b) {
                temp.setPixelColor(a, r, g, bl);
                a++;
            }

            if (show == true) 
                temp.show();
            //뭔가 추가하려고했는데 할게없네
            //아몰라
            
            /** @endcode Range_PixelChange */
       } 

};

//Resetting Servos
/**
 * @brief 
 * 반드시 Spider_Main.ino에서 servo.detach() 사용 후 선언할 것
 * ResetAll() 실행 시 오류 발생 가능성 유
 */
class ServoControl {
    private:
        const int servo_pin_private[4][3] = { {2, 3, 4}, {5, 6, 7}, {8, 9, 10}, {11, 12, 13} };
        Servo PrivateServo[4][3];
        #ifndef DEF_DELAY
            #define DEF_DELAY 500
        #endif

    public:
        ServoControl() {
            for (int i = 0; i < 4; i++) {
                for (int j = 0; j < 3; j++) {
                    PrivateServo[i][j].attach(servo_pin_private[i][j]);

                }
            }
        };



        
        /**
         * @brief 
         * reset으로 먼저 모든 서보를 되돌린 후 다시 degree값으로 모든 서보를 리셋
         * @param degree ; 해당값으로 모든 서보를 리셋
         * @param reset ; degree값으로 리셋하기 전 모든 모터를 해당값으로 변경
         */
        void ResetAll(int degree, int reset) { 
            if (reset) {
                for (int i = 0; i < 4; i++) {
                    for (int j = 0; j < 3; j++) {
                        PrivateServo[i][j].write(reset);
                        delay(DEF_DELAY);
                    }
                }
            }

            for (int i = 0; i < 4; i++) {
                for (int j = 0; j < 3; j++) {
                    PrivateServo[i][j].write(degree);
                    delay(DEF_DELAY);
                }
            }

        }

};
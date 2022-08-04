/*
 * 다음의 스파이더로봇 코드는 여러가지 라이브러리를 포함하고 있습니다.
 * 그러므로 외부의 라이브러리 파일을 직접 추가해주어야 합니다. 
 * 방법 : [스케치]-[라이브러리 포함하기]-[.ZIP파일 라이브러리 추가]
 */

/**
 * Test Case, Define "TEST"
 * 
 * Publish Case , define "PUBLIC"
 * 
 */


/**
 * @file Spider_Main.ino
 * @authors Unknown / Edited by Dev.Jh for private project
 * @brief 
 * Need understandence of "Servo, Arduino, FlexiTimer2, SerialCommand, NewPing, Adafruit_NeoPixel, main_led" header files.]
 * Use CommandHandler, for bluetooth module
 * check Reference with comment
 * @version 1.1
 * @date 2022-08-02
 * 
 * @copyright Copyright (c) 2022. Dev.Jh
 * 
 * @deprecated Line 93, ServoControl class callback = Remove this except test case
 * 
 */
#include <Servo.h>    //서보모터
#include <FlexiTimer2.h>//서보모터 타이머 기능, 게임패드를 활용을 위한 라이브러리 //  
#include <SerialCommand.h> //블루투스 모듈 사용을 위한 헤더파일
SerialCommand SCmd;   // 송수신 값 넣는 스토리지
#include <NewPing.h> // 개선된 초음파 센서 라이브러리 


#include <main_led.h> // 기본 LED 컨트롤 라이브러리 
#include <Adafruit_NeoPixel.h>

// 서보 핀 설정 
Servo servo[4][3]; //서보모터 핀 배열을 이용하여 값 설정
const int servo_pin[4][3] = { {2, 3, 4}, {5, 6, 7}, {8, 9, 10}, {11, 12, 13} };
// 로봇 사이즈 
const float length_a = 55;   
const float length_b = 77.5; 
const float length_c = 27.5; 
const float length_side = 100;   
const float z_absolute = -28;  

// 움직임 범위 
const float z_default = -50, z_up = -30,z_boot = z_absolute;
const float x_default = 62, x_offset = 0;   //62
const float y_start = 0, y_step = 40;       //40
const float y_default = x_default;

// 동작 속도 
volatile float site_now[4][3];    //각 다리 끝의 실시간 좌표
volatile float site_expect[4][3]; //각 다리 끝의 예상 좌표
float temp_speed[4][3];   //각 이동 전 재계산 해야하는 각 축의 속도
float move_speed;     //이동 속도
float speed_multiple = 1;  
const float spot_turn_speed = 4;
const float leg_move_speed = 8;
const float body_move_speed = 3;
const float stand_seat_speed = 1;
volatile int rest_counter;      //+1/0.02s, 자동 정지용

//함수의 매개 변수
const float KEEP = 255;

//pi값 정의
const float pi = 3.1415926;

// 회전 범위 (임시 지정)
const float temp_a = sqrt(pow(2 * x_default + length_side, 2) + pow(y_step, 2));
const float temp_b = 2 * (y_start + y_step) + length_side;
const float temp_c = sqrt(pow(2 * x_default + length_side, 2) + pow(2 * y_start + y_step + length_side, 2));
const float temp_alpha = acos((pow(temp_a, 2) + pow(temp_b, 2) - pow(temp_c, 2)) / 2 / temp_a / temp_b);
//회전을 위한 장소
const float turn_x1 = (temp_a - length_side) / 2;
const float turn_y1 = y_start + y_step / 2;
const float turn_x0 = turn_x1 - temp_b * cos(temp_alpha);
const float turn_y0 = temp_b * sin(temp_alpha) - turn_y1 - length_side;

// 초음파 센서
#define TRIGGER_PIN  A1
#define ECHO_PIN     A2
#define MAX_DISTANCE 200
boolean sonar_mode=false;
boolean freewalk_mode=false;
unsigned int avoid_dist=25;
NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);
        //sonar(Trig, Echo, 감지범위);



//기본 LED컨트롤 선언
LEDControl ledMain;


#define DEF_BRIGHTNESS 100
void setup()
{
  //디버깅을 위한 시리얼통신 시작
  Serial.begin(9600);
  Serial.println("Robot starts initialization");

  // 컨트롤을 하기 위해 정해놓은 값
  // 이 값을 받으면 그에 따른 방식으로 움직임
  // 움직임 제어 signal 0-6
  // w 0 1: 일어서기
  // w 0 0: 앉기
  // w 1 x: 앞으로 x번 가기
  // w 2 x: 뒤로 x번 가기
  // w 3 x: 오른쪽으로 x번 돌기
  // w 4 x: 왼쪽으로 x번 돌기
  // w 5 x: x번 손흔들기
  // w 6 x: x번 팔움직이기
  //위의 x는 움직이는 횟수를 뜻함
  // 아직 적용시키지 않은 signal
  // w 7 0: 초음파 센서를 이용한 거리 측정 300mm안에 물체가 있을시 손흔들기
  // w 8 0: 자율주행 모드
  // w 9 0: 엎드리기
  SCmd.addCommand("w", action_cmd);
  SCmd.setDefaultHandler(unrecognized);

  //서보모터 값 설정
  set_site(0, x_default - x_offset, y_start + y_step, z_boot);
  set_site(1, x_default - x_offset, y_start + y_step, z_boot);
  set_site(2, x_default + x_offset, y_start, z_boot);
  set_site(3, x_default + x_offset, y_start, z_boot);
  for (int i = 0; i < 4; i++)
  {
    for (int j = 0; j < 3; j++)
    {
      site_now[i][j] = site_expect[i][j];
    }
  }
  //서보모터 동작 코드 시작점
  FlexiTimer2::set(20, servo_service);
  FlexiTimer2::start();
  Serial.println("Servo service started");
  //초기 서보모터 설정
  servo_attach();
  Serial.println("Servos initialized");
  Serial.println("Robot initialization Complete");


  ledMain.begin();
  delay(300);
  ledMain.clear();

  if (freewalk_mode == true) {
      ledMain.All_PixelChange(162, 249,255, true);  // 자율주행 시작 시 파란색으로 설정
      ledMain.brightness(0, DEF_BRIGHTNESS); //default 
  } else {
      ledMain.All_PixelChange(0,150,0, true);  // 처음 시작 시 초록색으로 설정
      ledMain.brightness(0, DEF_BRIGHTNESS); //default 
  }


  pinMode(TRIGGER_PIN, OUTPUT); // trig에서 신호를 보내는 설정
  pinMode(ECHO_PIN, INPUT); // echo에서 신호를 받는 설정
}

//-서보모터 핀 할당
void servo_attach(void)
{
  for (int i = 0; i < 4; i++)
  {
    for (int j = 0; j < 3; j++)
    {
      servo[i][j].attach(servo_pin[i][j]);
      delay(100);
    }
  }
}

void servo_detach(void)
{
  for (int i = 0; i < 4; i++)
  {
    for (int j = 0; j < 3; j++)
    {
      servo[i][j].detach();
      delay(100);
    }
  }
}


void loop()
{
  SCmd.readSerial();  //BlueTooth
  if (freewalk_mode==true){
    // 자율 이동 모드
    freewalk(avoid_dist);
  } else if (sonar_mode==true) {
    // 초음파 수동제어 모드
    check_obstacle(avoid_dist);
  } 
}

void check_obstacle(unsigned int dist) {
  unsigned int ping_range;
    delay(50);
    Serial.print("Ping: ");
    Serial.print(sonar.ping_cm()); // ping 전송, 거리 계산 및 출력 (0 = 지정범위 밖의 거리)
    Serial.println("cm");
    ping_range=sonar.ping_cm();
    if ((ping_range<dist) and (ping_range!=0)) {

      ledMain.All_PixelChange(255, 0, 255, true); //장애물 감지 시 보라색 (#FF00FF) 로 변경
      ledMain.brightness(0, 200);

      // stand
      Serial.println("Wake up");
      stand();          
      // wave
      Serial.println("Shake");
      hand_shake(2);
      // turn
      //Serial.println("Turn");
      //turn_left(5);
      // sit
      Serial.println("Sit");
      sit();

      if (freewalk_mode == true) 
        ledMain.All_PixelChange(0,255,255,true);
      else
        ledMain.All_PixelChange(0, 255, 0, true);
      ledMain.brightness(0, DEF_BRIGHTNESS);
    }
}

/**
 * @brief do_test 함수 - 실행 사항 (각 항목 간 딜레이 2000ms (2s))
 * 1. 일어서기
 * 2. 앞으로 가기
 * 3. 뒤로 가기
 * 4. 왼쪽으로 둘기
 * 5. 오른쪽으로 돌기
 * 6. 손 흔들기 (hand_wave)
 * 7. 손 흔들기 (hand_shake)
 * 8. 몸 흔들기 (body_dance)
 * 9. 앉기 
 * 
 * 
 * @brief 개인적 추가 사항
 * do test 시작 시 0, 255, 255 (#00ffff) 로 LED 점등 with 최대밝기
 * 테스트 종료 시 0, 255, 0으로 재변경
 */
void do_test(void) 
{
  Serial.println("Test Start");

  ledMain.All_PixelChange(0, 255, 255, true); //컬러 변경 및 점등
  ledMain.brightness(0, 150); //밝기 변경 (decider 0 , means all)

  Serial.println("Stand");
  stand();
  delay(2000);
  Serial.println("Step forward");
  step_forward(5);
  delay(2000);
  Serial.println("Step back");
  step_back(5);
  delay(2000);
  Serial.println("Turn left");
  turn_left(5);
  delay(2000);
  Serial.println("Turn right");
  turn_right(5);
  delay(2000);
  Serial.println("Hand wave");
  hand_wave(3);
  delay(2000);
  Serial.println("Hand shake");
  hand_shake(3);
  delay(2000);
  Serial.println("Body dance");
  body_dance(10);
  delay(2000);    
  Serial.println("Sit");
  sit();
  delay(5000);

  Serial.println("Test End.");

  ledMain.All_PixelChange(0, 255, 0, true);
  ledMain.brightness(0, DEF_BRIGHTNESS);
}
  
// 움직임 제어 signal 0-6
// w 0 1: 일어서기
// w 0 0: 앉기
// w 1 x: 앞으로 x번 가기
// w 2 x: 뒤로 x번 가기
// w 3 x: 오른쪽으로 x번 돌기
// w 4 x: 왼쪽으로 x번 돌기
// w 5 x: x번 손흔들기
// w 6 x: x번 팔움직이기
//위의 x는 움직이는 횟수를 뜻함
// 아직 적용시키지 않은 signal
// w 7 0: 초음파 센서를 이용한 거리 측정 300mm안에 물체가 있을시 손흔들기
// w 8 0: 자율주행 모드
// w 9 0: 엎드리기

#define W_STAND_SIT    0
#define W_FORWARD      1
#define W_BACKWARD     2
#define W_LEFT         3
#define W_RIGHT        4
#define W_SHAKE        5
#define W_WAVE         6
#define W_SONAR        7
#define W_FREEWALK     8
#define W_LEG_INIT     9
void action_cmd(void)
{
  char *arg;
  int action_mode, n_step;
  Serial.println("Action:");
  arg = SCmd.next();
  action_mode = atoi(arg);
  arg = SCmd.next();
  n_step = atoi(arg);

  switch (action_mode)
  {
    case W_FORWARD:
      Serial.println("Step forward");
      if (!is_stand())
        stand();
      step_forward(n_step);
      break;
    case W_BACKWARD:
      Serial.println("Step back");
      if (!is_stand())
        stand();
      step_back(n_step);
      break;
    case W_LEFT:
      Serial.println("Turn left");
      if (!is_stand())
        stand();
      turn_left(n_step);
      break;
    case W_RIGHT:
      Serial.println("Turn right");
      if (!is_stand())
        stand();
      turn_right(n_step);
      break;
    case W_STAND_SIT:
      Serial.println("1:up,0:dn");
      if (n_step)
        stand();
      else
        sit();
      break;
    case W_SHAKE:
      Serial.println("Hand shake");
      hand_shake(n_step);
      break;
    case W_WAVE:
      Serial.println("Hand wave");
      hand_wave(n_step);
      break;
    case W_LEG_INIT:
      Serial.println("Legs init");
      legs_init();
      break;   
    case W_SONAR:
      Serial.println("Sonar mode");
      if (n_step>0)
        avoid_dist=n_step;
      do_sonar();
      break;    
    case W_FREEWALK:
      Serial.println("Freewalk mode");
      if (n_step>0)
        avoid_dist=n_step;
      do_freewalk();
      break;       
    default:
      Serial.println("Error");
      break;
  }
}

//일치하는 명령어가 없다면 다음 함수를 기본적으로 수행. 
void unrecognized(const char *command) {
  Serial.println("What?");
  ledMain.All_PixelChange(255, 0, 0, true); 
}

void freewalk(unsigned int dist) { //자율주행 직진 시작 시 하늘색으로 변경
   ledMain.All_PixelChange(0, 255, 255, true);
   ledMain.brightness(0, DEF_BRIGHTNESS); 
  
  unsigned int ping_range; //거리값을 받을 변수 설정
  ping_range=sonar.ping_cm(); // 초음파센서에서 받은 거리 값을 변수 ping_range에 저장
  // 20cm 전에 방향전환
  if ((ping_range<=dist) and (ping_range!=0)) { //초음파센서에서 받아들인 거리가 지정한 거리(dist)보다 작거나 같을 때, 
                                                //혹은 초음파 센서가 받아들이는 거리가 0이 아니라면 아래 문장들을 실행
    // 방향전환
    Serial.println("Turn Left");
    turn_left(5);
  } else {
    if (!is_stand())
        stand();
    Serial.println("Step forward");
    step_forward(2);
  }
}

// 자율주행모드
//자율주행모드 시 하늘색으로 변경, 자율주행모드 오프 시 초록색으로 변환
void do_freewalk(void) {
  if (freewalk_mode==false) {
    Serial.println("FreeWalk ON");
    freewalk_mode=true;
    ledMain.All_PixelChange(0, 255, 255, true);
    ledMain.brightness(0, DEF_BRIGHTNESS);
  } else {
    Serial.println("FreeWalk OFF");
    freewalk_mode=false;
    ledMain.All_PixelChange(0, 150, 0, true);
    ledMain.brightness(0, DEF_BRIGHTNESS);
  }
}

//초음파 모드 
void do_sonar(void){
  if (sonar_mode==false) {
    Serial.println("Sonar ON");
    sonar_mode=true;
  } else {
    Serial.println("Sonar OFF");
    sonar_mode=false;
  }
}


//다리 초기설정
void legs_init(void){
  
  //모든 서보모터 초기화
  move_speed = 8;
  for (int leg = 0; leg < 4; leg++)
  {
    set_site(leg, KEEP, 0, 90);
  }
  wait_all_reach();
}

//is_stand
bool is_stand(void)
{
  if (site_now[0][2] == z_default)
    return true;
  else
    return false;
}

/*
  - sit
  - blocking function
*/
void sit(void)
{
  move_speed = stand_seat_speed;
  for (int leg = 0; leg < 4; leg++)
  {
    set_site(leg, KEEP, KEEP, z_boot);
  }
  wait_all_reach();
}

/*
  - stand
  - blocking function
*/
void stand(void)
{
  move_speed = stand_seat_speed;
  for (int leg = 0; leg < 4; leg++)
  {
    set_site(leg, KEEP, KEEP, z_default);
  }
  wait_all_reach();
}


/*
  - spot turn to left
  - blocking function
  - parameter step steps wanted to turn
*/
void turn_left(unsigned int step)
{
  move_speed = spot_turn_speed;

  /*

  ledMain.Range_PixelChange(0, 1, 6, 255, 0, 0, true); //0 - 6픽셀 붉은색 점등;
  ledMain.Range_PixelChange(1, 1, 6, 255, 0, 0, true);

  */

  ledMain.All_PixelChange(255, 255, 0, true);

  while (step-- > 0)
  {
    if (site_now[3][1] == y_start)
    {
      //leg 3&1 move
      set_site(3, x_default + x_offset, y_start, z_up);
      wait_all_reach();

      set_site(0, turn_x1 - x_offset, turn_y1, z_default);
      set_site(1, turn_x0 - x_offset, turn_y0, z_default);
      set_site(2, turn_x1 + x_offset, turn_y1, z_default);
      set_site(3, turn_x0 + x_offset, turn_y0, z_up);
      wait_all_reach();

      set_site(3, turn_x0 + x_offset, turn_y0, z_default);
      wait_all_reach();

      set_site(0, turn_x1 + x_offset, turn_y1, z_default);
      set_site(1, turn_x0 + x_offset, turn_y0, z_default);
      set_site(2, turn_x1 - x_offset, turn_y1, z_default);
      set_site(3, turn_x0 - x_offset, turn_y0, z_default);
      wait_all_reach();

      set_site(1, turn_x0 + x_offset, turn_y0, z_up);
      wait_all_reach();

      set_site(0, x_default + x_offset, y_start, z_default);
      set_site(1, x_default + x_offset, y_start, z_up);
      set_site(2, x_default - x_offset, y_start + y_step, z_default);
      set_site(3, x_default - x_offset, y_start + y_step, z_default);
      wait_all_reach();

      set_site(1, x_default + x_offset, y_start, z_default);
      wait_all_reach();
    }
    else
    {
      //leg 0&2 move
      set_site(0, x_default + x_offset, y_start, z_up);
      wait_all_reach();

      set_site(0, turn_x0 + x_offset, turn_y0, z_up);
      set_site(1, turn_x1 + x_offset, turn_y1, z_default);
      set_site(2, turn_x0 - x_offset, turn_y0, z_default);
      set_site(3, turn_x1 - x_offset, turn_y1, z_default);
      wait_all_reach();

      set_site(0, turn_x0 + x_offset, turn_y0, z_default);
      wait_all_reach();

      set_site(0, turn_x0 - x_offset, turn_y0, z_default);
      set_site(1, turn_x1 - x_offset, turn_y1, z_default);
      set_site(2, turn_x0 + x_offset, turn_y0, z_default);
      set_site(3, turn_x1 + x_offset, turn_y1, z_default);
      wait_all_reach();

      set_site(2, turn_x0 + x_offset, turn_y0, z_up);
      wait_all_reach();

      set_site(0, x_default - x_offset, y_start + y_step, z_default);
      set_site(1, x_default - x_offset, y_start + y_step, z_default);
      set_site(2, x_default + x_offset, y_start, z_up);
      set_site(3, x_default + x_offset, y_start, z_default);
      wait_all_reach();

      set_site(2, x_default + x_offset, y_start, z_default);
      wait_all_reach();
    }
  }
  if(freewalk_mode == true)
    ledMain.All_PixelChange(0, 255, 255, true);
  else
    ledMain.All_PixelChange(0, 255, 0, true); //초록색 

  if (!ledMain.check_brightness(0, DEF_BRIGHTNESS)) //기본 밝기로
    ledMain.brightness(0, DEF_BRIGHTNESS);

//아니짆짜엏이가없넷ㅂ왜!=야!==가아니곳ㅄ비ㅣㅏㅇㄹㄴㅁㄹ이ㅏ멍리어이가없네
/*
  - spot turn to right
  - blocking function
  - parameter step steps wanted to turn
*/}

//우측으로 돌기
void turn_right(unsigned int step)
{

  /*

  ledMain.Range_PixelChange(0, 7, 12, 255, 0, 0, true); //7 - 12픽셀 붉은색 점등;
  ledMain.Range_PixelChange(1, 7, 12, 255, 0, 0, true);

  */

  ledMain.All_PixelChange(255,255,0, true);
  

  move_speed = spot_turn_speed;
  while (step-- > 0)
  {
    if (site_now[2][1] == y_start)
    {
      //leg 2&0 move
      set_site(2, x_default + x_offset, y_start, z_up);
      wait_all_reach();

      set_site(0, turn_x0 - x_offset, turn_y0, z_default);
      set_site(1, turn_x1 - x_offset, turn_y1, z_default);
      set_site(2, turn_x0 + x_offset, turn_y0, z_up);
      set_site(3, turn_x1 + x_offset, turn_y1, z_default);
      wait_all_reach();

      set_site(2, turn_x0 + x_offset, turn_y0, z_default);
      wait_all_reach();

      set_site(0, turn_x0 + x_offset, turn_y0, z_default);
      set_site(1, turn_x1 + x_offset, turn_y1, z_default);
      set_site(2, turn_x0 - x_offset, turn_y0, z_default);
      set_site(3, turn_x1 - x_offset, turn_y1, z_default);
      wait_all_reach();

      set_site(0, turn_x0 + x_offset, turn_y0, z_up);
      wait_all_reach();

      set_site(0, x_default + x_offset, y_start, z_up);
      set_site(1, x_default + x_offset, y_start, z_default);
      set_site(2, x_default - x_offset, y_start + y_step, z_default);
      set_site(3, x_default - x_offset, y_start + y_step, z_default);
      wait_all_reach();

      set_site(0, x_default + x_offset, y_start, z_default);
      wait_all_reach();
    }
    else
    {
      //leg 1&3 move
      set_site(1, x_default + x_offset, y_start, z_up);
      wait_all_reach();

      set_site(0, turn_x1 + x_offset, turn_y1, z_default);
      set_site(1, turn_x0 + x_offset, turn_y0, z_up);
      set_site(2, turn_x1 - x_offset, turn_y1, z_default);
      set_site(3, turn_x0 - x_offset, turn_y0, z_default);
      wait_all_reach();

      set_site(1, turn_x0 + x_offset, turn_y0, z_default);
      wait_all_reach();

      set_site(0, turn_x1 - x_offset, turn_y1, z_default);
      set_site(1, turn_x0 - x_offset, turn_y0, z_default);
      set_site(2, turn_x1 + x_offset, turn_y1, z_default);
      set_site(3, turn_x0 + x_offset, turn_y0, z_default);
      wait_all_reach();

      set_site(3, turn_x0 + x_offset, turn_y0, z_up);
      wait_all_reach();

      set_site(0, x_default - x_offset, y_start + y_step, z_default);
      set_site(1, x_default - x_offset, y_start + y_step, z_default);
      set_site(2, x_default + x_offset, y_start, z_default);
      set_site(3, x_default + x_offset, y_start, z_up);
      wait_all_reach();

      set_site(3, x_default + x_offset, y_start, z_default);
      wait_all_reach();
    }
  }

  if(freewalk_mode == true)
    ledMain.All_PixelChange(0, 255, 255, true);
  else
    ledMain.All_PixelChange(0, 255, 0, true); //초록색 

  if (!ledMain.check_brightness(0, DEF_BRIGHTNESS)) //기본 밝기로
    ledMain.brightness(0, DEF_BRIGHTNESS);
}

/*
  - go forward
  - blocking function
  - parameter step steps wanted to go
*/
void step_forward(unsigned int step)
{
  move_speed = leg_move_speed;
  while (step-- > 0)
  {
    if (site_now[2][1] == y_start)
    {
      //leg 2&1 move
      set_site(2, x_default + x_offset, y_start, z_up);
      wait_all_reach();
      set_site(2, x_default + x_offset, y_start + 2 * y_step, z_up);
      wait_all_reach();
      set_site(2, x_default + x_offset, y_start + 2 * y_step, z_default);
      wait_all_reach();

      move_speed = body_move_speed;

      set_site(0, x_default + x_offset, y_start, z_default);
      set_site(1, x_default + x_offset, y_start + 2 * y_step, z_default);
      set_site(2, x_default - x_offset, y_start + y_step, z_default);
      set_site(3, x_default - x_offset, y_start + y_step, z_default);
      wait_all_reach();

      move_speed = leg_move_speed;

      set_site(1, x_default + x_offset, y_start + 2 * y_step, z_up);
      wait_all_reach();
      set_site(1, x_default + x_offset, y_start, z_up);
      wait_all_reach();
      set_site(1, x_default + x_offset, y_start, z_default);
      wait_all_reach();
    }
    else
    {
      //leg 0&3 move
      set_site(0, x_default + x_offset, y_start, z_up);
      wait_all_reach();
      set_site(0, x_default + x_offset, y_start + 2 * y_step, z_up);
      wait_all_reach();
      set_site(0, x_default + x_offset, y_start + 2 * y_step, z_default);
      wait_all_reach();

      move_speed = body_move_speed;

      set_site(0, x_default - x_offset, y_start + y_step, z_default);
      set_site(1, x_default - x_offset, y_start + y_step, z_default);
      set_site(2, x_default + x_offset, y_start, z_default);
      set_site(3, x_default + x_offset, y_start + 2 * y_step, z_default);
      wait_all_reach();

      move_speed = leg_move_speed;

      set_site(3, x_default + x_offset, y_start + 2 * y_step, z_up);
      wait_all_reach();
      set_site(3, x_default + x_offset, y_start, z_up);
      wait_all_reach();
      set_site(3, x_default + x_offset, y_start, z_default);
      wait_all_reach();
    }
  }
}

/*
  - go back
  - blocking function
  - parameter step steps wanted to go
*/
void step_back(unsigned int step)
{
  ledMain.All_PixelChange(255, 0,0, true);
  
  move_speed = leg_move_speed;
  while (step-- > 0)
  {
    if (site_now[3][1] == y_start)
    {
      //leg 3&0 move
      set_site(3, x_default + x_offset, y_start, z_up);
      wait_all_reach();
      set_site(3, x_default + x_offset, y_start + 2 * y_step, z_up);
      wait_all_reach();
      set_site(3, x_default + x_offset, y_start + 2 * y_step, z_default);
      wait_all_reach();

      move_speed = body_move_speed;

      set_site(0, x_default + x_offset, y_start + 2 * y_step, z_default);
      set_site(1, x_default + x_offset, y_start, z_default);
      set_site(2, x_default - x_offset, y_start + y_step, z_default);
      set_site(3, x_default - x_offset, y_start + y_step, z_default);
      wait_all_reach();

      move_speed = leg_move_speed;

      set_site(0, x_default + x_offset, y_start + 2 * y_step, z_up);
      wait_all_reach();
      set_site(0, x_default + x_offset, y_start, z_up);
      wait_all_reach();
      set_site(0, x_default + x_offset, y_start, z_default);
      wait_all_reach();
    }
    else
    {
      //leg 1&2 move
      set_site(1, x_default + x_offset, y_start, z_up);
      wait_all_reach();
      set_site(1, x_default + x_offset, y_start + 2 * y_step, z_up);
      wait_all_reach();
      set_site(1, x_default + x_offset, y_start + 2 * y_step, z_default);
      wait_all_reach();

      move_speed = body_move_speed;

      set_site(0, x_default - x_offset, y_start + y_step, z_default);
      set_site(1, x_default - x_offset, y_start + y_step, z_default);
      set_site(2, x_default + x_offset, y_start + 2 * y_step, z_default);
      set_site(3, x_default + x_offset, y_start, z_default);
      wait_all_reach();

      move_speed = leg_move_speed;

      set_site(2, x_default + x_offset, y_start + 2 * y_step, z_up);
      wait_all_reach();
      set_site(2, x_default + x_offset, y_start, z_up);
      wait_all_reach();
      set_site(2, x_default + x_offset, y_start, z_default);
      wait_all_reach();
    }
  }

   if(freewalk_mode == true)
    ledMain.All_PixelChange(0, 255, 255, true);
  else
    ledMain.All_PixelChange(0, 255, 0, true); //초록색 

   ledMain.brightness(0, DEF_BRIGHTNESS);
}

// add by RegisHsu

void body_left(int i)
{
  set_site(0, site_now[0][0] + i, KEEP, KEEP);
  set_site(1, site_now[1][0] + i, KEEP, KEEP);
  set_site(2, site_now[2][0] - i, KEEP, KEEP);
  set_site(3, site_now[3][0] - i, KEEP, KEEP);
  wait_all_reach();
}

void body_right(int i)
{
  set_site(0, site_now[0][0] - i, KEEP, KEEP);
  set_site(1, site_now[1][0] - i, KEEP, KEEP);
  set_site(2, site_now[2][0] + i, KEEP, KEEP);
  set_site(3, site_now[3][0] + i, KEEP, KEEP);
  wait_all_reach();
}

//손 흔들기
void hand_wave(int i)
{
  float x_tmp;
  float y_tmp;
  float z_tmp;
  move_speed = 1;


  if (site_now[3][1] == y_start)
  {
    body_right(15);
    x_tmp = site_now[2][0];
    y_tmp = site_now[2][1];
    z_tmp = site_now[2][2];
    move_speed = body_move_speed;
    for (int j = 0; j < i; j++)
    {
      set_site(2, turn_x1, turn_y1, 50);
      wait_all_reach();
      set_site(2, turn_x0, turn_y0, 50);
      wait_all_reach();
    }
    set_site(2, x_tmp, y_tmp, z_tmp);
    wait_all_reach();
    move_speed = 1;
    body_left(15);
  }
  else
  {
    body_left(15);
    x_tmp = site_now[0][0];
    y_tmp = site_now[0][1];
    z_tmp = site_now[0][2];
    move_speed = body_move_speed;
    for (int j = 0; j < i; j++)
    {
      set_site(0, turn_x1, turn_y1, 50);
      wait_all_reach();
      set_site(0, turn_x0, turn_y0, 50);
      wait_all_reach();
    }
    set_site(0, x_tmp, y_tmp, z_tmp);
    wait_all_reach();
    move_speed = 1;
    body_right(15);
  }
}

//다리 흔들기
void hand_shake(int i)
{
  float x_tmp;
  float y_tmp;
  float z_tmp;
  move_speed = 1;

  if (site_now[3][1] == y_start)
  {
    body_right(15);
    x_tmp = site_now[2][0];
    y_tmp = site_now[2][1];
    z_tmp = site_now[2][2];
    move_speed = body_move_speed;
    for (int j = 0; j < i; j++)
    {
      set_site(2, x_default - 30, y_start + 2 * y_step, 55);
      wait_all_reach();
      set_site(2, x_default - 30, y_start + 2 * y_step, 10);
      wait_all_reach();
    }
    set_site(2, x_tmp, y_tmp, z_tmp);
    wait_all_reach();
    move_speed = 1;
    body_left(15);
  }
  else
  {
    body_left(15);
    x_tmp = site_now[0][0];
    y_tmp = site_now[0][1];
    z_tmp = site_now[0][2];
    move_speed = body_move_speed;
    for (int j = 0; j < i; j++)
    {
      set_site(0, x_default - 30, y_start + 2 * y_step, 55);
      wait_all_reach();
      set_site(0, x_default - 30, y_start + 2 * y_step, 10);
      wait_all_reach();
    }
    set_site(0, x_tmp, y_tmp, z_tmp);
    wait_all_reach();
    move_speed = 1;
    body_right(15);
  }
}

//머리 들기
void head_up(int i)
{
  set_site(0, KEEP, KEEP, site_now[0][2] - i);
  set_site(1, KEEP, KEEP, site_now[1][2] + i);
  set_site(2, KEEP, KEEP, site_now[2][2] - i);
  set_site(3, KEEP, KEEP, site_now[3][2] + i);
  wait_all_reach();
}

//머리 내리기
void head_down(int i)
{
  set_site(0, KEEP, KEEP, site_now[0][2] + i);
  set_site(1, KEEP, KEEP, site_now[1][2] - i);
  set_site(2, KEEP, KEEP, site_now[2][2] + i);
  set_site(3, KEEP, KEEP, site_now[3][2] - i);
  wait_all_reach();
}


//춤?
void body_dance(int i)
{
  float x_tmp;
  float y_tmp;
  float z_tmp;
  float body_dance_speed = 2;
  sit();
  move_speed = 1;
  set_site(0, x_default, y_default, KEEP);
  set_site(1, x_default, y_default, KEEP);
  set_site(2, x_default, y_default, KEEP);
  set_site(3, x_default, y_default, KEEP);
  wait_all_reach();
  //stand();
  set_site(0, x_default, y_default, z_default - 20);
  set_site(1, x_default, y_default, z_default - 20);
  set_site(2, x_default, y_default, z_default - 20);
  set_site(3, x_default, y_default, z_default - 20);
  wait_all_reach();
  move_speed = body_dance_speed;
  head_up(30);
  for (int j = 0; j < i; j++)
  {
    if (j > i / 4)
      move_speed = body_dance_speed * 2;
    if (j > i / 2)
      move_speed = body_dance_speed * 3;
    set_site(0, KEEP, y_default - 20, KEEP);
    set_site(1, KEEP, y_default + 20, KEEP);
    set_site(2, KEEP, y_default - 20, KEEP);
    set_site(3, KEEP, y_default + 20, KEEP);
    wait_all_reach();
    set_site(0, KEEP, y_default + 20, KEEP);
    set_site(1, KEEP, y_default - 20, KEEP);
    set_site(2, KEEP, y_default + 20, KEEP);
    set_site(3, KEEP, y_default - 20, KEEP);
    wait_all_reach();
  }
  move_speed = body_dance_speed;
  head_down(30);
}

void devjh(void) {
  Serial.println("Team 5 is <CENSORED>"); //이주혁 왔다감
}

/*
  - microservos service /timer interrupt function/50Hz
  - set site expected : 끝 점을 직선으로 이동
  - temp_speed[4][3]는 set site expected를 설정하기 전에 설정 되어야 하며, 끝 점이 일직선으로 이동하는지 확인하고 이동속도를 결정함.
   ---------------------------------------------------------------------------*/

//서보모터 구동코드
void servo_service(void)
{
  sei();
  static float alpha, beta, gamma;

  for (int i = 0; i < 4; i++)
  {
    for (int j = 0; j < 3; j++)
    {
      if (abs(site_now[i][j] - site_expect[i][j]) >= abs(temp_speed[i][j]))
        site_now[i][j] += temp_speed[i][j];
      else
        site_now[i][j] = site_expect[i][j];
    }

    cartesian_to_polar(alpha, beta, gamma, site_now[i][0], site_now[i][1], site_now[i][2]);
    polar_to_servo(i, alpha, beta, gamma);
  }

  rest_counter++;
}

/*
  - 끝점의 예상 지점 중 하나 설정
  - temp_speed[4][3]를 동시에 설정
  - non - blocking function
   ---------------------------------------------------------------------------*/
void set_site(int leg, float x, float y, float z)
{
  float length_x = 0, length_y = 0, length_z = 0;

  if (x != KEEP)
    length_x = x - site_now[leg][0];
  if (y != KEEP)
    length_y = y - site_now[leg][1];
  if (z != KEEP)
    length_z = z - site_now[leg][2];

  float length = sqrt(pow(length_x, 2) + pow(length_y, 2) + pow(length_z, 2));

  temp_speed[leg][0] = length_x / length * move_speed * speed_multiple;
  temp_speed[leg][1] = length_y / length * move_speed * speed_multiple;
  temp_speed[leg][2] = length_z / length * move_speed * speed_multiple;

  if (x != KEEP)
    site_expect[leg][0] = x;
  if (y != KEEP)
    site_expect[leg][1] = y;
  if (z != KEEP)
    site_expect[leg][2] = z;
}

/*
  - 끝점 중 하나가 예상 지점으로 이동하기를 기다림
  - blocking function
   ---------------------------------------------------------------------------*/
void wait_reach(int leg)
{
  while (1)
    if (site_now[leg][0] == site_expect[leg][0])
      if (site_now[leg][1] == site_expect[leg][1])
        if (site_now[leg][2] == site_expect[leg][2])
          break;
}

/*
  - 모든 끝점이 예상 지점으로 이동하기를 기다림
  - blocking function
   ---------------------------------------------------------------------------*/
void wait_all_reach(void)
{
  for (int i = 0; i < 4; i++)
    wait_reach(i);
}

/*
  - trans site from cartesian to polar
  - mathematical model 2/2
   ---------------------------------------------------------------------------*/
void cartesian_to_polar(volatile float &alpha, volatile float &beta, volatile float &gamma, volatile float x, volatile float y, volatile float z)
{
  //calculate w-z degree
  float v, w;
  w = (x >= 0 ? 1 : -1) * (sqrt(pow(x, 2) + pow(y, 2)));
  v = w - length_c;
  alpha = atan2(z, v) + acos((pow(length_a, 2) - pow(length_b, 2) + pow(v, 2) + pow(z, 2)) / 2 / length_a / sqrt(pow(v, 2) + pow(z, 2)));
  beta = acos((pow(length_a, 2) + pow(length_b, 2) - pow(v, 2) - pow(z, 2)) / 2 / length_a / length_b);
  //calculate x-y-z degree
  gamma = (w >= 0) ? atan2(y, x) : atan2(-y, -x);
  //trans degree pi->180
  alpha = alpha / pi * 180;
  beta = beta / pi * 180;
  gamma = gamma / pi * 180;
}

/*
  - trans site from polar to microservos
  - mathematical model map to fact
  - the errors saved in eeprom will be add  + eeprom; electrically erasable programmalbe read-only memory ; 전기 공급이 끊겨도 장기간 기억하는 비휘발성 장치
   ---------------------------------------------------------------------------*/
void polar_to_servo(int leg, float alpha, float beta, float gamma)
{
  if (leg == 0)
  {
    alpha = 90 - alpha;
    beta = beta;
    gamma += 90;
  }
  else if (leg == 1)
  {
    alpha += 90;
    beta = 180 - beta;
    gamma = 90 - gamma;
  }
  else if (leg == 2)
  {
    alpha += 90;
    beta = 180 - beta;
    gamma = 90 - gamma;
  }
  else if (leg == 3)
  {
    alpha = 90 - alpha;
    beta = beta;
    gamma += 90;
  }

  servo[leg][0].write(alpha);
  servo[leg][1].write(beta);
  servo[leg][2].write(gamma);
}

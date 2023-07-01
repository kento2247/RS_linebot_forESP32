#include <SPI.h>

#include "NFCcore8.h"

#define locate "yagami"
#define sound_pwm_channel 0
#define speaker_pin 25
#define blue_led_pin 14
#define red_led_pin 26
#define green_led_pin 27
#define SDA_pin 5
#define RST_pin 17

NFCcore8 nfc;  // mfrcクラスのインスタンス化
bool is_waiting = 0;
bool is_open = 0;
byte now_day = 0;

void setup() {
  // Initialize serial and wait for port to open:
  Serial.begin(115200);
  ledcSetup(sound_pwm_channel, 12000, 8);
  ledcAttachPin(speaker_pin, sound_pwm_channel);
  pinMode(blue_led_pin, OUTPUT);
  pinMode(red_led_pin, OUTPUT);
  pinMode(green_led_pin, OUTPUT);
  nfc.setup(SDA_pin, RST_pin, true);
  //ピン番号をアタッチ(SDA=5,RST=17)し、nfcクラスの初期設定完了。シリアルモニターにログ表示あり
  delay(100);


  https_init();
  now_day = https_get_day();
  // wpa2_init();
}

void loop() {
  is_waiting = 1;
  led_control(is_waiting, is_open, 0);

  String id = "";
  while (!nfc.waitTouch()) {
    if (int(millis() % 10000) == 0) {
      if (https_get_day() != now_day) {
        https_gas_get("0-0-0-0", 0);  //管理者権限で退勤
        is_open = 0;
        return;
      }
    }
    // wpa2_connecting_check();  //wpa2
  }
  is_waiting = 0;
  led_control(is_waiting, is_open, 1);
  if (is_open) scool_melody();
  else cure_music();
  Serial.println("touched");
  if(nfc.ApplePay) id="applepay";
  else id = String(nfc.uid[0]) + "-" + String(nfc.uid[1]) + "-" + String(nfc.uid[2]) + "-" + String(nfc.uid[3]);


  // String result = https_gas_get_redirect(id, !is_open);
  // int indexFrom = result.indexOf("result=");
  // return;


  https_gas_get(id, !is_open);
  // https_aws_post(id, !is_open);

  is_open = !is_open;
}
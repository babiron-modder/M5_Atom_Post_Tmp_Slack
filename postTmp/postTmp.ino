#include "M5Atom.h"
#include "Adafruit_Sensor.h"
#include <Adafruit_BMP280.h>
#include <Wire.h>
#include "SHT3X.h"
#include <ArduinoJson.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>

const char *SSID            = "[MY_WIFI_SSID]";
const char *PASSWORD        = "[MY_WIFI_PASSWORD]";
const char *SLACK_POST_URL  = "https://hooks.slack.com/services/[MY_SLACK_BOT_POST_URL]";
const unsigned long TIME    = 600000;  // 10分ごとに温度を送信

struct count_timer{
  unsigned long now;
  unsigned long past;
  unsigned long count;
} time_clock;

SHT3X sht30;
char tmp_text[64];
float tmp;
bool data_sending_now;


void makeJson(char* out, size_t len,char* color, char* text){
  DynamicJsonDocument doc(255);

  JsonArray attachments = doc.createNestedArray("attachments");

  JsonObject attachments_0 = attachments.createNestedObject();
  JsonArray attachments_0_mrkdwn_in = attachments_0.createNestedArray("mrkdwn_in");
  attachments_0_mrkdwn_in.add("text");
  attachments_0["text"] = text;
  attachments_0["color"] = color;

  serializeJson(doc, out, len);
}


bool postMessage(char* color, char* text){
  M5.dis.drawpix(0, 0xf00000);
  char buffer[255];
  HTTPClient http;
  int timeout_counter = 0;
  bool send_is_ok = false;

  // ============  送信するデータの生成  ============
  makeJson(buffer, sizeof(buffer), color, text);

  // ============  データを送信  ============
  while(1){
    if( ( ++timeout_counter ) > 10000 ){
       break;
    }
    if(http.begin(SLACK_POST_URL)){
      http.addHeader("Content-Type", "application/json");
      int status_code = http.POST((uint8_t*)buffer, strlen(buffer));
      if( status_code == 200 ){
        Stream* resp = http.getStreamPtr();
      }
      http.setReuse(false);
      http.end();
      send_is_ok = true;
	  break;
    }
    delay(1);
  }

  M5.dis.clear();
  return send_is_ok;
}


void setup(){
  // ============  M5Stackの初期化  ============
  M5.begin(false,false,true);
  Wire.begin(26, 32);
  M5.dis.drawpix(0, CRGB::Black);

  // ============  wifiのセットアップ  ============
  WiFi.begin(SSID, PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    M5.dis.drawpix(0, 0x0000f0);
    delay(50);
  }
  M5.dis.clear();

  // ============  変数の初期化  ============
  time_clock.past = 0;
  time_clock.now = 0;
  time_clock.count = TIME;
  data_sending_now = false;
}



void loop() {

  M5.update();

  // ============  wifi接続の確認  ============
  if(WiFi.status() != WL_CONNECTED){
    WiFi.begin(SSID, PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
      M5.dis.drawpix(0, 0x0000f0);
      delay(50);
    }
    M5.dis.clear();
  }

  // ============  一応セマフォ  ============
  if(data_sending_now == false){
    data_sending_now = true;

    // ============  経過時間の確認  ============
    time_clock.now = millis();

    if(time_clock.past < time_clock.now){
      time_clock.count += (time_clock.now - time_clock.past);
    }else{
      // オーバフローが起きた場合
      time_clock.count += (time_clock.now +(4294967295 - time_clock.past));
    }
    time_clock.past = time_clock.now;

    // ============  温度の取得  ============
    if(sht30.get()==0){
      tmp = sht30.cTemp;
    }

    // ============  データの送信  ============
    if(time_clock.count > TIME){
      snprintf(tmp_text, 64,"[OK] %2.2f ℃", tmp);

      if(postMessage("good",tmp_text)){
        time_clock.count = time_clock.count - TIME;
      }
    }

    data_sending_now = false;
  }

  // ============   ボタン操作  ============
  if(M5.Btn.wasPressed()){
    postMessage("good","[OK] 99.99 ℃ <!channel>(test)");
  }

  delay(50);
}

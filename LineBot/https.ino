#include <HTTPClient.h>
#include <HTTPSRedirect.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include "time.h"
#include <Preferences.h>

#define JST 3600 * 9

#define https_ssid "aterm-059a4d-g"  // your network https_ssid (name of wifi network)
#define https_password "ooya1192"    // your network https_password

HTTPSRedirect* client = nullptr;
WiFiMulti wifiMulti;
Preferences preferences;
const String gas_url = "https://script.google.com/macros/s/AKfycbwue4Oknk-LHHqZJK1WstXRZMsCLAAkwQB1wuUjddJWoKyXX3W3PPVMSTZ9lBIF-tkR/exec";
const String aws_host = "https://jjh8maycrk.execute-api.ap-northeast-1.amazonaws.com";
const String aws_path = "/dev/clock-in";

void https_init() {
  // Serial.print("Attempting to connect to https_ssid: ");
  // Serial.println(https_ssid);
  // WiFi.begin(https_ssid, https_password);
  wifiMulti.addAP(https_ssid, https_password);
  wifiMulti.addAP("kentoのiPhone 14 Pro", "ja1yaeja1yae");


  Serial.println("Connecting Wifi...");
  while (wifiMulti.run() != WL_CONNECTED) {
    delay(10);
  }
  if (wifiMulti.run() == WL_CONNECTED) {
    Serial.println("");
    Serial.println("WiFi connected");
    char wifi_ssid[37] = {};
    preferences.begin("nvs.net80211", true);
    preferences.getBytes("sta.ssid", wifi_ssid, sizeof(wifi_ssid));
    Serial.printf("sta.SSID : %s\n", &wifi_ssid[4]);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  }

  // byte faild_counter = 0;
  // while (WiFi.status() != WL_CONNECTED) {
  //   Serial.print(".");
  //   // wait 1 second for re-trying
  //   delay(1000);
  //   faild_counter++;
  //   if (faild_counter == 10) {
  //     ESP.restart();
  //   }
  // }

  // if (WiFi.status() == WL_CONNECTED) {
  //   Serial.print("Connected to ");
  //   Serial.println(https_ssid);
  // }
  configTime(JST, 0, "ntp.nict.jp", "ntp.jst.mfeed.ad.jp");
}

void https_gas_get(String id, bool status) {
  struct tm timeInfo;
  getLocalTime(&timeInfo);
  String time_str = String(timeInfo.tm_year + 1900) + "-" + String(timeInfo.tm_mon + 1) + "-" + String(timeInfo.tm_mday) + "T" + String(timeInfo.tm_hour) + ":" + String(timeInfo.tm_min) + ":" + String(timeInfo.tm_sec);
  String url =
    gas_url + "?app=nfc_reader&locate=" + String(locate) + "&id=" + String(id) + "&status=" + String(status) + "&time=" + time_str;
  HTTPClient http;
  http.begin(url);
  int httpCode = http.GET();
  http.end();
}

String https_gas_get_redirect(String id, bool status) {
  struct tm timeInfo;
  getLocalTime(&timeInfo);
  String time_str = String(timeInfo.tm_year + 1900) + "-" + String(timeInfo.tm_mon + 1) + "-" + String(timeInfo.tm_mday) + "T" + String(timeInfo.tm_hour) + ":" + String(timeInfo.tm_min) + ":" + String(timeInfo.tm_sec);
  String url =
    gas_url + "?locate=" + String(locate) + "&id=" + String(id) + "&status=" + String(status) + "&time=" + time_str;
  const char* host = "script.google.com";
  client = new HTTPSRedirect(443);
  String body;
  client->setInsecure();
  client->setPrintResponseBody(true);
  client->setContentTypeHeader("application/json");
  client->connect(host, 443);
  client->GET(url, host);
  body = client->getResponseBody();
  delete client;
  client = nullptr;
  Serial.println("end");
  return body;
}

void https_aws_post(String id, bool status) {
  struct tm timeInfo;
  getLocalTime(&timeInfo);
  String time_str = String(timeInfo.tm_year + 1900) + "-" + String(timeInfo.tm_mon + 1) + "-" + String(timeInfo.tm_mday) + "T" + String(timeInfo.tm_hour) + ":" + String(timeInfo.tm_min) + ":" + String(timeInfo.tm_sec);
  HTTPClient http;
  http.begin(aws_host + aws_path);
  String data_json = "{locate:" + String(locate) + ",id:" + String(id) + ",status:" + String(status) + ",time:" + time_str + "}";
  int httpCode = http.POST((uint8_t*)data_json.c_str(), data_json.length());
  if (httpCode > 0) {
    Serial.printf("[HTTP] POST... code: %d\n", httpCode);
    if (httpCode == HTTP_CODE_OK) {
      String payload = http.getString();
      Serial.println(payload);
    }
  } else {
    Serial.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
  }
  http.end();
}

byte https_get_day() {
  struct tm timeInfo;
  getLocalTime(&timeInfo);
  return timeInfo.tm_mday;
}
// 2020/07/02更新

#include <MFRC522.h>
#include <SPI.h>

#include "Arduino.h"

//#define RST_PIN 9            // NFC RSTピン(SPI通信時には設定必要)
//#define CS_PIN 10            // NFC SDAピン(SPI通信時には設定必要

class NFCcore8 {
  //関数一覧
 public:
  void init();  //変数を全て初期化
  bool setup(byte SDA, byte RST, bool SelectDumpInfo);  // SDA=CS=SS
  bool waitTouch(unsigned int miliTimeLimit);
  bool waitTouch();
  bool blockRead(byte block);
  bool blockWrite(byte block);
  bool sectorRead(byte sector);
  bool sectorWrite(byte sector);
  bool blockReset(byte block);
  bool writeName(String FirstName, String LastName);
  String readName();  //読み取り失敗時、"failed"を返す
  bool get_cardInfo();  //現在のカード内のブロック1、基本情報を確認
  void show_cardInfo();
  bool set_cardInfo();
  bool
  updateSavedTime();  //ブロック1。呼び出し前にpublic内のDate[]を更新する必要あり。

 private:
  void Disconnect();
  void setKeyA(byte block);
  void setKeyB(byte block);
  bool authA(byte block);
  bool authB(byte block);

  //変数、配列一覧
 public:
  const byte NFCcoreVersion = 8;  //このプログラムのバージョンを規定
  byte cardNFCcoreVersion = 0;  // versionAuth()でアップデート
  String userName = "";         //初期値
  bool ApplePay =
      false;  // waitTouch()の際に更新」。ApplePay端末がかざされたかの確認
  byte blockReadData[16];
  byte sectorReadData[3][16];
  byte blockWriteData[16];
  byte sectorWriteData[3][16];
  byte uid[4];
  int Date[6] = {
      0, 1, 1, 0,
      0, 0};  //デフォルト[年,月,日,時,分,秒]!!ただし年は1970年からの経過年数を用いる
  unsigned int organizationID;
  byte userID;
  byte optionalStatus;
  unsigned int totalRecordTime;
  unsigned int optionalValue;

 protected:
  const byte miliInterval =
      50;  // waitTouch()内でのカードの読み取り頻度。推奨50ms
  bool dumpInfo =
      true;  //各関数内でSerial.print()を行うか否か。組み込みの際は表示なしが推奨
  bool touched = false;  //カードがかざされているか否か。waitTouch()内で更新
  byte keyA[16][6] = {
      {255, 255, 255, 255, 255, 255}, {255, 255, 255, 255, 255, 255},
      {255, 255, 255, 255, 255, 255}, {255, 255, 255, 255, 255, 255},
      {255, 255, 255, 255, 255, 255}, {255, 255, 255, 255, 255, 255},
      {255, 255, 255, 255, 255, 255}, {255, 255, 255, 255, 255, 255},
      {255, 255, 255, 255, 255, 255}, {255, 255, 255, 255, 255, 255},
      {255, 255, 255, 255, 255, 255}, {255, 255, 255, 255, 255, 255},
      {255, 255, 255, 255, 255, 255}, {255, 255, 255, 255, 255, 255},
      {255, 255, 255, 255, 255, 255}, {255, 255, 255, 255, 255, 255}};
  byte keyB[16][6] = {
      {255, 255, 255, 255, 255, 255}, {255, 255, 255, 255, 255, 255},
      {255, 255, 255, 255, 255, 255}, {255, 255, 255, 255, 255, 255},
      {255, 255, 255, 255, 255, 255}, {255, 255, 255, 255, 255, 255},
      {255, 255, 255, 255, 255, 255}, {255, 255, 255, 255, 255, 255},
      {255, 255, 255, 255, 255, 255}, {255, 255, 255, 255, 255, 255},
      {255, 255, 255, 255, 255, 255}, {255, 255, 255, 255, 255, 255},
      {255, 255, 255, 255, 255, 255}, {255, 255, 255, 255, 255, 255},
      {255, 255, 255, 255, 255, 255}, {255, 255, 255, 255, 255, 255}};
  MFRC522::StatusCode status;  //ステータス変数を定義
  MFRC522::MIFARE_Key KeyA = {
    keyByte : {255, 255, 255, 255, 255, 255}
  };  //デフォルトキー
  MFRC522::MIFARE_Key KeyB = {
    keyByte : {255, 255, 255, 255, 255, 255}
  };  //デフォルトキー
  MFRC522::PICC_Type piccType;
  MFRC522 mfrc522;  // RC522と接続
};

#include "NFCcore8.h"


void NFCcore8::init() {
  //public変数
  cardNFCcoreVersion = 0;
  userName = "";
  ApplePay = false;
  for (byte i = 0; i < 16; i++) blockReadData[i] = 0;
  for (byte i = 0; i < 3; i++) {
    for (byte n = 0; n < 16; n++) sectorReadData[i][n] = 0;
  }
  for (byte i = 0; i < 16; i++) blockWriteData[i] = 0;
  for (byte i = 0; i < 3; i++) {
    for (byte n = 0; n < 16; n++) sectorWriteData[i][n] = 0;
  }
  for (byte i = 0; i < 4; i++) uid[i] = 0;
  dumpInfo = true;
  touched = false;
  for (byte i = 0; i < 16; i++) {
    for (byte n = 0; n < 6; n++) keyA[i][n] = 255;
  }
  for (byte i = 0; i < 16; i++) {
    for (byte n = 0; n < 6; n++) keyB[i][n] = 255;
  }
  Date[0] = 1970;
  Date[1] = 1;
  Date[2] = 1;
  Date[3] = 0;
  Date[4] = 0;
  Date[5] = 0;
}


bool NFCcore8::setup(byte CS, byte RST, bool SelectDumpInfo) {
  Serial.println("\nsetup for using NFC library");
  while (!Serial);  //ProMicroなどのATMEGA32Uチップマイコンのみ必要(他はあっても問題ない)
  SPI.begin();
  Serial.println(F("  begin SPI com with RC522 module"));
  delay(10);
  Serial.println(F("  initialize nfc_PCD"));
  mfrc522.PCD_Init(CS, RST);  //PCD (Proximity Coupling Device)の初期化
  Serial.print(F("  dumpInfo function > "));
  if (SelectDumpInfo) {
    Serial.println(F("ON"));
    Serial.print(F("  "));
    mfrc522.PCD_DumpVersionToSerial();
    //    Serial.print(F(">>self perform test result > "));
    //    Serial.println(mfrc522.PCD_PerformSelfTest());
    //    Serial.print(F(">>antenna's gain > "));
    //    Serial.println(mfrc522.PCD_GetAntennaGain());
  }
  else Serial.println(F("OFF"));
  dumpInfo = SelectDumpInfo;
  //  return mfrc522.PCD_PerformSelfTest();
  return true;
}


bool NFCcore8::waitTouch(unsigned int miliTimeLimit) {
  unsigned int baseTime = millis();
  Disconnect();
  delay(100);
  while (! mfrc522.PICC_IsNewCardPresent() || ! mfrc522.PICC_ReadCardSerial())  {
    delay(miliInterval);    //変更可能(タッチ判定の更新速度)
    if ((millis() - baseTime) < miliTimeLimit) return false;
  }
  String strUID;
  for (byte i = 0; i < 4; i++) {
    uid[i] = mfrc522.uid.uidByte[i];
    strUID += String(uid[i], HEX) += " ";
  }
  if (String(uid[0], HEX) == "8") ApplePay = true;
  else ApplePay = false;
  piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
  if (dumpInfo) {
    Serial.println(F("NFCtag touched"));
    Serial.print(F("  card UID is "));
    Serial.println(strUID);
    if (ApplePay) Serial.println(F("  >this card type is ApplePay"));
    Serial.print(F("  PICC type: "));
    Serial.print(piccType);
    Serial.print(" >> ");
    Serial.println(mfrc522.PICC_GetTypeName(piccType));
  }
  touched = true;
  return true;
}


bool NFCcore8::waitTouch() {
  unsigned int baseTime = millis();
  Disconnect();
  delay(100);
  while (! mfrc522.PICC_IsNewCardPresent() || ! mfrc522.PICC_ReadCardSerial()) {
    delay(miliInterval);    //変更可能(タッチ判定の更新速度)
  }
  String strUID;
  for (byte i = 0; i < 4; i++) {
    uid[i] = mfrc522.uid.uidByte[i];
    strUID += String(uid[i]) += ",";
  }
  if (String(uid[0], HEX) == "8") ApplePay = true;
  else ApplePay = false;
  piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
  if (dumpInfo) {
    Serial.println(F("NFCtag touched"));
    Serial.print(F("  card UID is "));
    Serial.println(strUID);
    if (ApplePay) Serial.println(F("  >this card type is ApplePay"));
    Serial.print(F("  PICC type: "));
    Serial.print(piccType);
    Serial.print(" >> ");
    Serial.println(mfrc522.PICC_GetTypeName(piccType));
  }
  touched = true;
  return true;
}


bool NFCcore8::blockRead(byte block) {
  byte temporaryBlockData[18];  //仕様上18byte必要なので、16バイトにトリミングるる前の元データ
  if (dumpInfo) {
    Serial.println(F("begin blockRead"));
    Serial.print(F("  use sector "));
    Serial.println((byte)(block / 4));
    Serial.print(F("  block "));
    Serial.println(block);
  }
  if (!authA(block)) return false;
  for (byte i = 0; i < 18; i++) temporaryBlockData[i] = 0; //配列の中身の初期化
  byte  size = 18;
  status = (MFRC522::StatusCode)mfrc522.MIFARE_Read(block, temporaryBlockData, &size);
  if (status != MFRC522::STATUS_OK) {
    if (!dumpInfo) return false;
    Serial.println(F(">>blockRead failed"));
    Serial.println();
    return false;
  }
  if (!dumpInfo) return true;
  Serial.println(F(">>blockRead is in success"));
  Serial.print(F("  result: "));
  for (byte i = 0; i < 16; i++) {
    blockReadData[i] = temporaryBlockData[i];
    Serial.print(blockReadData[i]);
    Serial.print(", ");
  }
  Serial.println();
  return true;
}


bool NFCcore8::blockWrite(byte block) {
  if (block % 4 == 3) return false;
  updateSavedTime();
  if (dumpInfo) {
    Serial.println(F("begin blockWrite"));
    Serial.print(F("  using sector "));
    Serial.println((byte)(block / 4));
    Serial.print(F("  using block "));
    Serial.println(block);
    Serial.print(F("  write data is "));
    for (byte i = 0; i < 16; i++) {
      Serial.print(blockWriteData[i]);
      Serial.print(", ");  //csv形式変換
    }
    Serial.println();
  }
  if (!authB(block)) return false;
  status = (MFRC522::StatusCode)mfrc522.MIFARE_Write(block, blockWriteData, 16);
  if (status != MFRC522::STATUS_OK) {
    if (!dumpInfo) return false;
    Serial.println(F(">>blockWrite failed"));
    Serial.println();
    return false;
  }
  if (!dumpInfo) return true;
  Serial.println(F("blockWrite is in success"));
  Serial.println();
  return true;
}


bool NFCcore8::sectorRead(byte sector) {
  byte temporaryBlockData2[18];
  if (dumpInfo) {
    Serial.println(F("begin sectorRead"));
    Serial.print(F("  use sector "));
    Serial.println(sector);
  }
  if (!authA(sector * 4)) return false;
  for (byte j = 0; j < 3; j++) {
    for (byte i = 0; i < 16; i++) sectorReadData[j][i] = 0; //配列の中身の初期化
  }
  for (byte i = 0; i < 3; i++) {
    byte  size = 18;
    status = (MFRC522::StatusCode)mfrc522.MIFARE_Read((byte)(sector * 4 + i), temporaryBlockData2, &size);
    if (status != MFRC522::STATUS_OK) {
      if (!dumpInfo) return false;
      Serial.println(F(">>sectorRead failed"));
      Serial.println();
      return false;
    }
    for (byte j = 0; j < 16; j++) {
      sectorReadData[i][j] = temporaryBlockData2[j];  //トリミング
    }
  }
  if (!dumpInfo) return true;
  Serial.println(F(">>sectorRead is in success"));
  Serial.println(F("  result: "));
  for (byte j = 0; j < 3; j++) {
    Serial.print(F("        "));
    for (byte i = 0; i < 16; i++) {
      Serial.print(sectorReadData[j][i]);
      Serial.print(", ");
    }
    Serial.println();
  }
  return true;
}


bool NFCcore8::sectorWrite(byte sector) {
  updateSavedTime();
  if (dumpInfo) {
    Serial.println(F("begin sectorWrite"));
    Serial.print(F("  using sector "));
    Serial.println(sector);
    Serial.print(F("  write data "));
    for (byte j = 0; j < 3; j++) {
      for (byte i = 0; i < 16; i++) {
        Serial.print(sectorWriteData[j][i]);
        Serial.print(", ");  //csv形式変換
      }
      Serial.println();
    }
  }
  if (!authB(sector * 4)) return false;
  for (byte j = 0; j < 3; j++) {
    for (byte i = 0; i < 16; i++) {
      blockWriteData[i] = sectorWriteData[j][i];
    }
    status = (MFRC522::StatusCode)mfrc522.MIFARE_Write((byte)(sector * 4 + j), blockWriteData, 16);
    if (status != MFRC522::STATUS_OK) {
      if (!dumpInfo) return false;
      Serial.println(F(">>sectorWrite failed"));
      Serial.println();
      return false;
    }
  }
  if (!dumpInfo) return true;
  Serial.println(F("sectorWrite is in success"));
  Serial.println();
  return true;
}


void NFCcore8::Disconnect() {
  touched = false;
  mfrc522.PICC_HaltA();             // 接続中の端末との通信を停止
  mfrc522.PCD_StopCrypto1();        // 端末とのPCDの暗号化を停止
}


void NFCcore8::setKeyA(byte block) {
  byte sector = block / 4;
  for (byte i = 0; i < 6; i++) {
    KeyA.keyByte[i] = keyA[sector][i];
  }
  if (dumpInfo) {
    Serial.print(F("    >>set keyA data for sector "));
    Serial.println(sector);
    Serial.print(F("      use KeyA: "));
    for (byte i = 0; i < 6; i++) {
      Serial.print(KeyA.keyByte[i]);
      Serial.print(", ");
    }
    Serial.println();
  }
}


void NFCcore8::setKeyB(byte block) {
  byte sector = block / 4;
  for (byte i = 0; i < 6; i++) {
    KeyB.keyByte[i] = keyB[sector][i];
  }
  if (dumpInfo) {
    Serial.print(F("    >>set keyB data for sector "));
    Serial.println(sector);
    Serial.print(F("      use KeyB: "));
    for (byte i = 0; i < 6; i++) {
      Serial.print(KeyB.keyByte[i]);
      Serial.print(", ");
    }
    Serial.println();
  }
}


bool NFCcore8::authA(byte block) {
  byte sector = block / 4;
  byte trailerBlock   = sector * 4 + 3;  //そのセクターの認証キーが格納されているブロック番号
  piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
  if (dumpInfo) {
    Serial.println(F("  >>start authenticate typeA(read)"));
    Serial.print(F("    piccType: "));
    Serial.println(piccType);
    if (piccType == MFRC522::PICC_TYPE_MIFARE_UL) Serial.println(F(">>skiped authA (because MifareUL)"));
  }
  if (piccType != MFRC522::PICC_TYPE_MIFARE_UL) {
    setKeyA(block);
    status = (MFRC522::StatusCode)mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &KeyA, &(mfrc522.uid));
    if (status != MFRC522::STATUS_OK) {
      if (!dumpInfo) return false;
      Serial.println(F("  >>authA() failed"));
      Serial.println();
      return false;
    }
    else return true;  //MifareULではなく、認証に成功した場合
  }
  return true;  //MifareULの場合
}


bool NFCcore8::authB(byte block) {
  byte sector = block / 4;
  byte trailerBlock   = sector * 4 + 3;  //そのセクターの認証キーが格納されているブロック番号
  piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
  if (dumpInfo) {
    Serial.println(F("  >>start authenticate typeB(write)"));
    Serial.print(F("    piccType: "));
    Serial.println(piccType);
    if (piccType == MFRC522::PICC_TYPE_MIFARE_UL) Serial.println(F(">>skiped authB (because MifareUL)"));
  }
  if (piccType != MFRC522::PICC_TYPE_MIFARE_UL) {
    setKeyB(block);
    status = (MFRC522::StatusCode)mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_B, trailerBlock, &KeyB, &(mfrc522.uid));
    if (status != MFRC522::STATUS_OK) {
      if (!dumpInfo) return false;
      Serial.println(F("  >>authB() failed"));
      Serial.println();
      return false;
    }
    else return true;  //MifareULではなく、認証に成功した場合
  }
  return true;  //MifareULの場合
}


bool NFCcore8::blockReset(byte block) {
  for (byte i = 0; i < 16; i++) {
    blockWriteData[i] = 0;
  }
  if (blockWrite(block)) return true;
  else return false;
}


bool NFCcore8::writeName(String FirstName, String LastName) {
  if (dumpInfo)Serial.println(F("NFC begin write user_name data"));
  FirstName.replace("\n", "");
  LastName.replace("\n", "");
  String Name = FirstName + '-' + LastName; //keio-taroのようにハイフンを入れる
  if (dumpInfo)Serial.print("  " + Name + " ");

  if (Name.length() >= 16) {
    if (dumpInfo)Serial.println(F(">>writeName() failed (name data is too long)"));   //16要素を超えてしまう場合
    return false;
  }
  else {
    for (byte i = Name.length(); i < 17; i++) {
      Name += "";  //名前が16字以下の場合、残りスペースを0で上書き（前のデータが残っている可能性をなくすため）
    }
    Name.getBytes(blockWriteData, 16);  //Name変数をASCIIコード配列に変換 (書き込み先配列,要素数)
    if (dumpInfo) {
      Serial.print(F(">>convert to ASCIIcode : ["));
      for (byte i = 0; i < 16; i++) {
        Serial.print(blockWriteData[i]);
        Serial.print(",");
      }
      Serial.println("]");
    }
    if (!blockWrite(2)) {   //書き込み失敗
      if (dumpInfo)Serial.println(F(">>writeName() failed"));
      return false;
    }
    else {
      if (dumpInfo)Serial.println(F(">>writeName() is in success"));
      return true;
    }
  }
}


String NFCcore8::readName() {
  if (dumpInfo)Serial.println(F("NFC begin read userName data"));
  if (blockRead(2) == true) {
    if (dumpInfo)Serial.print(F("  raw name data = "));
    userName = "";  //初期化
    for (byte i = 0; i < 16; i++) {
      if (blockReadData[i] == 0) break;       //変換完了
      Serial.print(blockReadData[i]);
      userName += char(blockReadData[i]); //1文字ずつ変換して、文字列に付け加えていく
      Serial.print(F(" "));
    }
    if (dumpInfo)Serial.print(F("\n>>convert result: "));
    Serial.println(userName);
    return userName;  //変換したデータを呼び出し元に返送する
  }
  else {
    if (dumpInfo)Serial.println(F(">>readName() failed"));
    Serial.println();
    return String (F("failed"));
  }
}


bool NFCcore8::get_cardInfo() { //現在のバージョン
  if (!blockRead(1)) return false;
  cardNFCcoreVersion = blockReadData[0];
  for (byte i = 0; i < 6; i++) Date[i] =  blockReadData[i + 1];  //カード内に保存されていた最終記録時刻
  organizationID = makeWord(blockReadData[7], blockReadData[8]);
  userID = blockReadData[9];
  optionalStatus = blockReadData[11];
  totalRecordTime = makeWord(blockReadData[12], blockReadData[13]);
  optionalValue = makeWord(blockReadData[14], blockReadData[15]);
  return true;
}

void NFCcore8::show_cardInfo() {
  Serial.println("user name : " + userName);
  Serial.println("card version : " + String(cardNFCcoreVersion));
  Serial.println("time data in nfc chip : " + String(Date[0] + 1900) + "/" + String(Date[1]) + "/" + String(Date[2]) + " " +
                 String(Date[3]) + ":" + String(Date[4]) + ":" + String(Date[5]));  //時刻を表示
  Serial.println("organizationID : " + String(organizationID));
  Serial.println("user id : " + String(userID));
  Serial.println("attendance status : " + String(optionalStatus));
  Serial.println("total tecord time : " + String(totalRecordTime));
  Serial.println("total attend time : " + String(optionalValue));
}


bool NFCcore8::updateSavedTime() {
  byte WriteUserData[16];
  if (!blockRead(1)) return false;
  cardNFCcoreVersion = blockReadData[0];
  WriteUserData[0] = cardNFCcoreVersion;
  for (byte i = 1; i < 7; i++) WriteUserData[i] = Date[i - 1]; //updateSavedTime()呼び出し前にDate[]が更新されていれば、カード情報に反映される
  for (byte i = 7; i < 16; i++) WriteUserData[i] = blockReadData[i];  //元の情報をコピー

  if (dumpInfo) {
    Serial.println(F(">>begin updateSavedTime()"));
    Serial.println(F("  using sector 0"));
    Serial.println(F("  using block 1"));
    Serial.print(F("  write data  "));
    for (byte i = 0; i < 16; i++) {
      Serial.print(WriteUserData[i]);
      Serial.print(", ");  //csv形式変換
    }
    Serial.println("\n  now card format version :" + String(cardNFCcoreVersion));
    Serial.println("  set : " + String(Date[0]) + "/" + String(Date[1]) + "/" + String(Date[2]) + " " + String(Date[3]) + ":" + String(Date[4]) + ":" + String(Date[5]));
  }
  if (!authB(1)) return false;
  status = (MFRC522::StatusCode)mfrc522.MIFARE_Write(1, WriteUserData, 16);
  if (status != MFRC522::STATUS_OK) {
    if (!dumpInfo) return false;
    Serial.println(F(">>blockWrite failed"));
    Serial.println();
    return false;
  }
  if (!dumpInfo) return true;
  Serial.println(F("blockWrite is in success"));
  Serial.println();
  return true;
}

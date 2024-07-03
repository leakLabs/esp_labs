#include <Wire.h>
#include <LiquidCrystal_AIP31068_I2C.h>
#include <SimpleKeypad.h>
#include <EEPROM.h>
#include "m8toArd.h"
#include "NetBytes.h"
#include <SoftSPIB.h>
#include <EEPROM.h>

////////////////////////////////////
#define ROW_PINS   \
  {                \
    D3, D4, D5, D6 \
  }
#define COLUMN_PINS \
  {                 \
    B5, B4, B3      \
  }
#define DISPLAY_ADRESS 0x3E
////////////////////////////////////

// Настройка LCD дисплея
LiquidCrystal_AIP31068_I2C lcd(DISPLAY_ADRESS, 16, 2);

// Настройка клавиатуры
const byte ROWS = 4;
const byte COLS = 3;
char keys[ROWS * COLS] = {
    '1', '2', '3',
    '4', '5', '6',
    '7', '8', '9',
    '*', '0', '#'};
byte rowPins[ROWS] = ROW_PINS;
byte colPins[COLS] = COLUMN_PINS;
SimpleKeypad keypad = SimpleKeypad(keys, rowPins, colPins, ROWS, COLS);
enum State
{
  STEP1_INIT,
  STEP2_PASS,
  STEP2_USERINPUT,
  USER_IDIOT_STATE,
  STEP3_SETTINGS,
  STEP4_DOWNLOAD,
  CONN_LOST,
  PACKAGE_LOSS_STATE
} state;

enum DownloadResult
{
  COMPLETE,
  PACK_LOSS,
  CN_LOST,
  USER_,
  WRONG_PASS
};

byte pinAttempts = 4;
byte attemptN = 0;
char openPass[5];
byte packRec;
bool isWaitingPing = false;
unsigned long lastPing = 0;
bool downloadStart = false;

byte packageCount;            // = 100; // в eeprom
uint16_t minPackageTime;      // = 5;
uint16_t maxPackageTime;      // = 100;
unsigned long answerWaitTime; // = 2000;
uint16_t alarm;               // = 50;
uint16_t longAlarm;           // = 300;
byte attemptsCount;           // = 4;
byte step2Error;              // = 12;
uint16_t pingPeriod;          // = 750; // итого - 17 байт

void step1Init();
void step2Pass();
void step2UserInput();
void userIdiotHandler();
void step3Settings();
void step4Download();
void connectLostHandler();
void packageLossHandler();
void eepromSend();
void eepromDataRead();
void startMenu();
bool connectionOK();
void writeHistory(DownloadResult, byte, byte);
void readHistory(DownloadResult &, byte &, byte &);

void setup()
{
  // Инициализация LCD
  lcd.init();
  startMenu();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Initialization");
  Serial.begin(9600);
  delay(100);
  state = STEP1_INIT;
  eepromDataRead();
}

void loop()
{
  switch (state)
  {
  case STEP1_INIT:
    step1Init();
    break;
  case STEP2_PASS:
    step2Pass();
    break;
  case STEP2_USERINPUT:
    step2UserInput();
    break;
  case USER_IDIOT_STATE:
    userIdiotHandler();
    break;
  case STEP3_SETTINGS:
    step3Settings();
    break;
  case STEP4_DOWNLOAD:
    step4Download();
    break;
  case CONN_LOST:
    connectLostHandler();
    break;
  case PACKAGE_LOSS_STATE:
    packageLossHandler();
    break;
  }
}

void step1Init()
{
  downloadStart = false;
  attemptN = 0;
  while (!(Serial.available() == 1 && Serial.read() == CONN_ASK))
  {
  }

  Serial.write(CONN_ANS);
  eepromSend();
  state = STEP2_PASS;
}

void step2Pass()
{
  while (Serial.available() != 5) // ждём пока что-нибудь придёт
  {
  }
  Serial.read(); // читаем код
  Serial.readBytes(openPass, 4);
  openPass[4] = '\0';
  state = STEP2_USERINPUT;
}

void step2UserInput()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Code:");
  lcd.print(openPass);
  lcd.setCursor(0, 1);
  lcd.print("Your pin:"); // 9
  char closePass[4];
  byte pos = 0;
  char key = 0;
  isWaitingPing = false;

  while (key != '#')
  {
    key = keypad.getKey();
    switch (key)
    {
    case 0:
    case '#':
      if (pos != 4)
        key = 0;
      break;

    case '*':
      if (pos > 0)
      {
        lcd.setCursor(8 + pos, 1);
        lcd.print(' ');
        lcd.setCursor(8 + pos, 1);
        pos--;
      }
      break;

    default:
      if (pos < 4)
      {
        lcd.print('*');
        closePass[pos++] = key;
      }
      break;
    }

    if (Serial.available() == 1 && Serial.peek() == USER_IDIOT) // обработка ошибки
    {
      Serial.read();
      state = USER_IDIOT_STATE;
      return;
    }

    if (!connectionOK())
      return;
  }

  isWaitingPing = false;

  if (key == '#')
  {
    Serial.write(CLOSE_PIN);
    Serial.write(closePass, 4);
  }

  while (Serial.available() != 1)
  {
  }
  if (Serial.peek() == PIN_CORRECT)
  {
    Serial.read();
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print("Access Granted  ");
    state = STEP3_SETTINGS;
    return;
  }
  else if (Serial.peek() == PIN_WRONG)
  {
    Serial.read();
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Access Denied   ");
    lcd.setCursor(0, 1);
    lcd.print("Attempts: ");
    lcd.print(pinAttempts - ++attemptN);
    unsigned long tmpTime = millis();
    while (millis() - tmpTime < 2000)
    {
      if (!connectionOK())
        return;
    }
    lcd.clear();
    return;
  }
  else
  {
    writeHistory(WRONG_PASS, 0, 0);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Blocked");
    lcd.setCursor(0, 1);
    lcd.print("No more attempts");
    delay(2000);
    lcd.clear();
    lcd.print("Initialization");
    state = STEP1_INIT;
    return;
  }
}

void userIdiotHandler()
{
  writeHistory(USER_, 0, 0);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Wrong order!");
  lcd.setCursor(0, 1);
  lcd.print("Try again");
  delay(2000);
  lcd.clear();
  lcd.print("Initialization");
  state = STEP1_INIT;
}

void step3Settings()
{
  while (Serial.available() == 0 || (Serial.available() != 0 && Serial.peek() == CONN_OK))
  {
    if (!connectionOK())
      return;
  }

  if (Serial.peek() == USER_IDIOT)
  {
    Serial.read();
    state = USER_IDIOT_STATE;
    return;
  }

  Serial.read();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Progress:     0%"); // 12-3-1
  lcd.setCursor(0, 1);
  lcd.print("Freq:   100.0MHz"); // 8-5-3
  bool end = false;
  isWaitingPing = false;

  while (!end)
  {
    if (Serial.available() >= 2 || (Serial.available() == 1 && (Serial.peek() == USER_IDIOT || Serial.peek() == SETT_COMLETE)))
    {
      switch (Serial.read())
      {
      case USER_IDIOT:
      {
        state = USER_IDIOT_STATE;
        return;
      }

      case SETT_PERC:
      {
        byte tmp = Serial.read();
        lcd.setCursor(12, 0);
        lcd.print("   ");
        lcd.setCursor((tmp == 100) ? 12 : ((tmp > 9) ? 13 : 14), 0);
        lcd.print(tmp);
        break;
      }

      case SETT_FRQN:
      {
        byte tmp = Serial.read();
        lcd.setCursor(8, 1);
        lcd.print("   ");
        lcd.setCursor(8, 1);
        lcd.print(100 + tmp / 10);
        lcd.print('.');
        lcd.print(tmp % 10);
        break;
      }

      case SETT_COMLETE:
      {
        end = true;
        lcd.clear();
      }
      }
    }
    if (!connectionOK())
      return;
  }

  state = STEP4_DOWNLOAD;
}

void step4Download()
{
  byte progrBar = 1; // 1-14
  packRec = 0;
  while (Serial.available() != 1)
  {
  }

  if (Serial.peek() == USER_IDIOT)
  {
    Serial.read();
    state = USER_IDIOT_STATE;
    return;
  }
  Serial.read();

  lcd.print(char(255));
  lcd.setCursor(15, 0);
  lcd.print(char(255));
  lcd.setCursor(7, 1); // 8-1-7
  lcd.print("0/");
  lcd.print(packageCount);
  unsigned long lastReceiveTime = millis();
  downloadStart = true;

  while (packRec < packageCount)
  {
    if (Serial.available() == 2 || (Serial.available() == 1 && (Serial.peek() == USER_IDIOT || Serial.peek() == PACKAGE_LOSS)))
    {
      if (Serial.peek() == USER_IDIOT)
      {
        writeHistory(USER_, packRec, packageCount);
        Serial.read();
        state = USER_IDIOT_STATE;
        return;
      }
      else if (Serial.peek() == PACKAGE_LOSS)
      {
        Serial.read();
        state = PACKAGE_LOSS_STATE;
        return;
      }
      Serial.read();
      Serial.read();
      Serial.write(PACK_REC);
      packRec++;
      lastReceiveTime = millis();

      byte tmp = map(packRec, 0, packageCount, 1, 15);
      if (progrBar < tmp)
      {
        lcd.setCursor(progrBar, 0);
        for (; progrBar < tmp; progrBar++)
          lcd.print(char(255));
      }
      lcd.setCursor(5, 1);
      lcd.print("   ");
      lcd.setCursor((packRec >= 100) ? 5 : ((packRec >= 10) ? 6 : 7), 1);
      lcd.print(packRec);
    }
    if (millis() - lastReceiveTime > answerWaitTime / 2 + maxPackageTime)
    {
      state = CONN_LOST;
      return;
    }
  }

  while (Serial.available() != 1)
  {
  }

  if (Serial.peek() == USER_IDIOT)
  {
    writeHistory(USER_, packRec, packageCount);
    Serial.read();
    state = USER_IDIOT_STATE;
    return;
  }

  Serial.read();
  lcd.setCursor(0, 0);
  lcd.print("                ");
  lcd.setCursor(3, 0);
  lcd.print("Completed!");
  writeHistory(COMPLETE, packRec, packageCount);

  state = (State)20;
}

void connectLostHandler()
{
  if (downloadStart)
  {
    writeHistory(CN_LOST, packRec, packageCount);
    lcd.setCursor(0, 0);
    lcd.print("                ");
  }
  else
  {
    writeHistory(CN_LOST, 0, 0);
    lcd.clear();
  }

  lcd.setCursor(1, 0);
  lcd.print("Connection Lost");
  state = STEP1_INIT;
}

void packageLossHandler()
{
  lcd.setCursor(0, 0);
  lcd.print("                ");
  lcd.setCursor(3, 0);
  lcd.print("Package loss");
  writeHistory(PACK_LOSS, packRec, packageCount);
  state = STEP1_INIT;
}

void eepromSend()
{
  Serial.write(packageCount);
  Serial.write((byte *)(&minPackageTime), 2);
  Serial.write((byte *)(&maxPackageTime), 2);
  Serial.write((byte *)(&answerWaitTime), 4);
  Serial.write((byte *)(&alarm), 2);
  Serial.write((byte *)(&longAlarm), 2);
  Serial.write(attemptsCount);
  Serial.write(step2Error);
}

void eepromDataRead()
{
  EEPROM.get(0, packageCount);
  EEPROM.get(1, minPackageTime);
  EEPROM.get(3, maxPackageTime);
  EEPROM.get(5, answerWaitTime);
  EEPROM.get(9, alarm);
  EEPROM.get(11, longAlarm);
  EEPROM.get(13, attemptsCount);
  EEPROM.get(14, step2Error);
  EEPROM.get(15, pingPeriod);
}

void startMenu()
{
  lcd.setCursor(0, 0);
  lcd.print("1 for history");
  lcd.setCursor(0, 1);
  lcd.print("2 for connect");
  char key = keypad.getKey();
  while (key != '1' && key != '2')
  {
    key = keypad.getKey();
  }

  if (key == '1')
  {
    byte packageCountTmp, packRecTmp;
    DownloadResult drt;
    readHistory(drt, packRecTmp, packageCountTmp);

    lcd.clear();
    lcd.setCursor(0, 0);
    const char *res;
    switch (drt)
    {
    case COMPLETE:
      res = "Completed";
      break;
    case PACK_LOSS:
      res = "Package loss";
      break;
    case CN_LOST:
      res = "Connection Lost";
      break;
    case USER_:
      res = "Wrong order";
      break;
    case WRONG_PASS:
      res = "Wrong password";
      break;
    }
    lcd.print(res);
    if (packageCountTmp != 0)
    {
      lcd.setCursor(0, 1);
      lcd.print(packRecTmp);
      lcd.print('/');
      lcd.print(packageCountTmp);
    }
  }

  while (key != '2')
  {
    key = keypad.getKey();
  }
}

bool connectionOK()
{
  if (Serial.available() > 0 && Serial.peek() == CONN_OK)
  {
    Serial.read();
    isWaitingPing = false;
    lastPing = millis();
  }

  if (!isWaitingPing && millis() - lastPing > pingPeriod)
  {
    Serial.write(CHECK_CONN);
    lastPing = millis();
    isWaitingPing = true;
  }

  if (isWaitingPing && millis() - lastPing > answerWaitTime)
  {
    isWaitingPing = false;
    state = CONN_LOST;
    return false;
  }
  return true;
}

void writeHistory(DownloadResult res, byte received, byte count)
{
  EEPROM.put(17, res);
  EEPROM.put(19, received);
  EEPROM.put(20, count);
}

void readHistory(DownloadResult &drt, byte &packRecTmp, byte &packageCountTmp)
{
  EEPROM.get(17, drt);
  EEPROM.get(19, packRecTmp);
  EEPROM.get(20, packageCountTmp);
}

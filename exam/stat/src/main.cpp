#include <Arduino.h>
#include "m8toArd.h"
#include "ShiftLed.h"
#include <EncButton.h>
#include <Stepper.h>
#include <EEPROM.h>
#include "NetBytes.h"

/////////////////////
#define SWITCH1 D5
#define SWITCH2 D6
#define SWITCH3 D7
#define SHIFT_DATA D2
#define SHIFT_CLOCK D4
#define SHIFT_LATCH D3
#define UART_SPEED 9600
#define OPEN_PASS "1234"
#define CLOSE_PASS "5678"
#define ENC1_DT B4
#define ENC1_CLK B3
#define ENC2_DT B2
#define ENC2_CLK B1
#define STEPPER_STEPS 32
#define STEPPER_PIN1 C5
#define STEPPER_PIN2 C4
#define STEPPER_PIN3 C3
#define STEPPER_PIN4 C2
#define SOUND B0
/////////////////////

enum State
{
    START,
    STEP1,
    STEP2,
    STEP3,
    CONNECT_LOST_ERROR,
    PACKAGE_LOSS_ERROR,
    WRONG_ORDER,
    WRONG_PASSWORDS,
    END
};

State state = START;
ShiftLed shiftLeds(SHIFT_LATCH, SHIFT_CLOCK, SHIFT_DATA);
Encoder enc1(ENC1_DT, ENC1_CLK);
Encoder enc2(ENC2_DT, ENC2_CLK);
Stepper stepper(STEPPER_STEPS, STEPPER_PIN4, STEPPER_PIN2, STEPPER_PIN3, STEPPER_PIN1);
byte settingsPerc;
unsigned long lastPing;

byte packageCount;
uint16_t minPackageTime;
uint16_t maxPackageTime;
unsigned long answerWaitTime = 2000;
uint16_t alarm;
uint16_t longAlarm = 300;
byte attemptsCount;
byte step2Error; // итого - 15 байт

void powOn();
void step1();
bool checkSwitches(byte sw1, byte sw2, byte sw3);
void step2();
void step3();
void handleBaseError();
void handlePackageLossError();
void handleWrongPasssError();
void setRandom();
void readSettings();
bool connectionOK();
void lastPingRst();

///////////////////////////////////

void setup()
{
    setRandom();
    pinMode(SWITCH1, INPUT_PULLUP);
    pinMode(SWITCH2, INPUT_PULLUP);
    pinMode(SWITCH3, INPUT_PULLUP);
    pinMode(B0, OUTPUT);
    stepper.setSpeed(10);
    Serial.begin(UART_SPEED);
}

void loop()
{
    switch (state)
    {
    case START:
        powOn();
        break;

    case STEP1:
        step1();
        break;

    case STEP2:
        step2();
        break;

    case STEP3:
        step3();
        break;

    case CONNECT_LOST_ERROR:
        handleBaseError();
        break;

    case WRONG_ORDER:
        Serial.write(USER_IDIOT);
        handleBaseError();
        break;

    case PACKAGE_LOSS_ERROR:
        handlePackageLossError();
        break;

    case WRONG_PASSWORDS:
        handleWrongPasssError();
        break;

    case END:
        break;

    default:
        break;
    }
}

///////////////////////////////////

void powOn()
{
    while (digitalRead(SWITCH1) == LOW || digitalRead(SWITCH2) == LOW || digitalRead(SWITCH3) == LOW) // ожидание выключения всех тумблеров
    {
        shiftLeds.setAll(RED).send();
        delay(longAlarm);
        shiftLeds.setAll(LEDOFF).send();
        delay(longAlarm);
    }
    shiftLeds.setAll(RED).send();
    state = STEP1;
}

void step1()
{
    while (checkSwitches(HIGH, HIGH, HIGH))
    {
    }

    if (!(checkSwitches(LOW, HIGH, HIGH)))
    {
        state = WRONG_ORDER;
        return;
    }

    Serial.write(CONN_ASK); // Опрос мобильного модуля на подкючение

    unsigned long tmpTime = millis();
    while (Serial.available() != 16 && millis() - tmpTime < answerWaitTime)
    {
        if (!(checkSwitches(LOW, HIGH, HIGH)))
        {
            state = WRONG_ORDER;
            return;
        }
    }
    if (!(Serial.available() == 16 && Serial.read() == CONN_ANS)) // ToDo
    {
        state = WRONG_ORDER;
        return;
    }
    readSettings();
    shiftLeds.setGroup(1, YELLOW).send();

    Serial.write(OPEN_PIN);
    Serial.write(OPEN_PASS, 4); // Отправка пароля
    byte inBuffer[4];
    byte isValid;
    byte attCounter = 0;
    lastPingRst();

    do // Получение введенных паролей
    {
        while (Serial.available() < 5)
        {
            if (!connectionOK())
            {
                return;
            }

            if (!(checkSwitches(LOW, HIGH, HIGH)))
            {
                state = WRONG_ORDER;
                return;
            }
        }

        if (Serial.peek() == CHECK_CONN)
        {
            Serial.read();
            Serial.write(CONN_OK);
        }
        Serial.read();
        Serial.readBytes(inBuffer, 4);
        isValid = 1;

        for (byte i = 0; i < 4 && isValid == 1; ++i)
            if (inBuffer[i] != CLOSE_PASS[i])
                isValid = 0;

        if (isValid == 0)
        {
            if (++attCounter >= attemptsCount)
            {
                state = WRONG_PASSWORDS;
                return;
            }
            Serial.write(PIN_WRONG);
            digitalWrite(SOUND, HIGH);
            delay(longAlarm);
            digitalWrite(SOUND, LOW);
            lastPingRst();
        }
        else
            Serial.write(PIN_CORRECT);
    } while (isValid == 0);

    shiftLeds.setGroup(1, GREEN).send();

    state = STEP2;
}

bool checkSwitches(byte sw1, byte sw2, byte sw3)
{
    return digitalRead(SWITCH1) == sw1 && digitalRead(SWITCH2) == sw2 && digitalRead(SWITCH3) == sw3;
}

void step2()
{
    lastPingRst();
    while (checkSwitches(LOW, HIGH, HIGH))
    {
        if (!connectionOK())
        {
            return;
        }
    }

    if (!(checkSwitches(LOW, LOW, HIGH)))
    {
        state = WRONG_ORDER;
        return;
    }

    Serial.write(SETTINGS_START);

    bool updated = false;

    byte targetStep = random(0, STEPPER_STEPS);
    byte steps = 0;
    byte stepPos = 0;
    byte progress1 = 0;

    byte progress2 = 0; // 0 - 50
    byte frqnc = 0;     // 0 - 200
    byte targetFr = random(1, 200);

    lastPingRst();
    while (checkSwitches(LOW, LOW, HIGH))
    {
        if (!connectionOK())
            return;

        enc1.tick();
        enc2.tick();
        if (enc1.turn())
        {
            if (steps == 0 && enc1.dir() == -1)
                steps = STEPPER_STEPS - 1;
            else
                steps = (steps + enc1.dir()) % STEPPER_STEPS;

            if (steps == 0)
                progress1 = 0;
            else if (steps < targetStep)
                progress1 = map(steps, 0, targetStep, 0, 50);
            else
                progress1 = map(steps, targetStep, STEPPER_STEPS - 1, 50, 0);

            updated = true;
        }

        if (stepPos != steps)
        {
            byte rawDiff = (stepPos < steps) ? (steps - stepPos) : (stepPos - steps);
            int8_t offset = ((rawDiff < STEPPER_STEPS / 2 && steps > stepPos) || (rawDiff > STEPPER_STEPS / 2 && steps < stepPos)) ? 1 : -1;
            stepper.step(offset);

            if (stepPos == 0 && offset == -1)
                stepPos = STEPPER_STEPS - 1;
            else
                stepPos = (stepPos + offset) % STEPPER_STEPS;
        }

        if (enc2.turn())
        {
            if (enc2.dir() == 1 && frqnc < 200)
                frqnc++;
            else if (enc2.dir() == -1 && frqnc > 0)
                frqnc--;

            if (frqnc < targetFr)
                progress2 = map(frqnc, 0, targetFr, 0, 50);
            else
                progress2 = map(frqnc, targetFr, 200, 50, 0);

            Serial.write(SETT_FRQN);
            Serial.write(frqnc);
            updated = true;
        }

        if (updated)
        {
            Serial.write(SETT_PERC);
            Serial.write(progress1 + progress2);
            updated = false;
            byte color;

            if (progress1 + progress2 == 100)
                color = GREEN;
            else if (progress1 + progress2 + step2Error >= 100)
                color = YELLOW;
            else
                color = RED;

            if (color != shiftLeds.getGroupColor(2))
                shiftLeds.setGroup(2, color).send();
        }
    }

    if (!(checkSwitches(LOW, LOW, LOW)))
    {
        state = WRONG_ORDER;
        return;
    }

    settingsPerc = progress1 + progress2;
    Serial.write(SETT_COMLETE);
    state = STEP3;
}

void step3()
{
    shiftLeds.setGroup(3, YELLOW).send();

    Serial.write(START_DOWLOAD);

    byte sended = 0;
    byte sendErrors = 0;
    byte timeout;
    unsigned long tmpTime;

    while (sended < packageCount && checkSwitches(LOW, LOW, LOW))
    {
        timeout = random(minPackageTime, maxPackageTime);
        delay(timeout);
        if (random(0, 100) < settingsPerc)
        {
            Serial.write(NEW_PACK);
            Serial.write(sended);
            tmpTime = millis();
            while (Serial.available() == 0 && millis() - tmpTime < answerWaitTime / 2 + maxPackageTime)
            {
            }

            if (Serial.available() != 0 && Serial.read() == PACK_REC)
            {
                sended++;
                sendErrors = 0;
                digitalWrite(SOUND, HIGH);
                delay(alarm);
                digitalWrite(SOUND, LOW);
                delay(alarm);
            }
            else
            {
                state = CONNECT_LOST_ERROR;
                return;
            }
        }
        else
        {
            sendErrors++;
        }

        if (sendErrors >= 3)
        {
            state = PACKAGE_LOSS_ERROR;
            return;
        }
    }

    if (!(checkSwitches(LOW, LOW, LOW)))
    {
        state = WRONG_ORDER;
        return;
    }

    Serial.write(WORK_COMPLETE);

    shiftLeds.setGroup(3, GREEN).send();
    state = END;
}

void handleBaseError()
{
    for (byte i = 0; i < 3; i++)
    {
        digitalWrite(SOUND, HIGH);
        shiftLeds.setAll(RED).send();
        delay(longAlarm);
        digitalWrite(SOUND, LOW);
        shiftLeds.setAll(LEDOFF).send();
        delay(longAlarm);
    }
    while (Serial.available())
        Serial.read();

    state = START;
}

void handlePackageLossError()
{
    Serial.write(PACKAGE_LOSS);
    handleBaseError();
}

void handleWrongPasssError()
{
    Serial.write(PIN_BLOCK);
    handleBaseError();
}

void setRandom()
{
    unsigned long tmp;
    EEPROM.get(0, tmp); // 0-4
    randomSeed(tmp++);
    EEPROM.put(0, tmp);
}

void readSettings()
{
    packageCount = Serial.read();
    Serial.readBytes((byte *)(&minPackageTime), 2);
    Serial.readBytes((byte *)(&maxPackageTime), 2);
    Serial.readBytes((byte *)(&answerWaitTime), 4);
    Serial.readBytes((byte *)(&alarm), 2);
    Serial.readBytes((byte *)(&longAlarm), 2);
    attemptsCount = Serial.read();
    step2Error = Serial.read();
}

bool connectionOK()
{
    if (Serial.available() != 0 && Serial.peek() == CHECK_CONN)
    {
        Serial.read();
        Serial.write(CONN_OK);
        lastPing = millis();
        return true;
    }
    else if (millis() - lastPing > answerWaitTime)
    {
        state = CONNECT_LOST_ERROR;
        return false;
    }
    else
    {
        return true;
    }
}

void lastPingRst()
{
    lastPing = millis();
}

#include <Arduino.h>
#include <Joystick.h>

Joystick_ Joystick(JOYSTICK_DEFAULT_REPORT_ID,
                   JOYSTICK_TYPE_GAMEPAD, 6,
                   JOYSTICK_DEFAULT_HATSWITCH_COUNT,
                   false, false, false, false, false, false,
                   false, false, false, false, false);

const int KEY_PORT_COUNT = 6;

const int KEY_ARRAY_OFFSET[2] = {0, 8};
const int KEY_PORT_BEGIN[2] = {2, A0};
const int KEY_ROW_OFFSET[2] = {0, 4};
const int KEY_COLUMN_OFFSET[2] = {2, 0};

const int MAX_DEBOUNCE = 10;

int buttons[16];

int btnPressed(int id)
{
  return (buttons[id] == MAX_DEBOUNCE) ? 1 : 0;
}

int btnReleased(int id)
{
  return (buttons[id] == 0) ? 1 : 0;
}

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);

  // Initialize Joystick Library
  Joystick.begin();
}

struct joyAction
{
  bool button;
  int id;
  int direction;
};

joyAction btn2joy[16] = {
    {true, 0, 0},    // 0
    {true, 5, 0},    // 1
    {true, 4, 0},    // 2
    {true, 1, 0},    // 3
    {false, 1, 270}, // 4
    {false, 1, 180}, // 5
    {false, 1, 90},  // 6
    {false, 1, 0},   // 7
    {false, 0, 0},   // 8
    {false, 0, 90},  // 9
    {false, 0, 180}, // 10
    {false, 0, 270}, // 11
    {true, 0, 0},    // 12
    {true, 2, 0},    // 13
    {true, 3, 0},    // 14
    {true, 0, 0}     // 15
};

int getHatAngle(int hatId)
{
  int hatDirs[4] = {-1, -1, -1, -1};
  int hatDirsIdx = 0;

  for (int i = 0; i < 16; ++i)
  {
    if (!btn2joy[i].button && btn2joy[i].id == hatId && btnPressed(i))
      hatDirs[hatDirsIdx++] = btn2joy[i].direction;
  }

  if (hatDirsIdx == 0)
    return JOYSTICK_HATSWITCH_RELEASE;

  if (hatDirsIdx == 1)
    return hatDirs[0];

  if (hatDirsIdx == 2)
  {
    if (hatDirs[0] == 0 && hatDirs[1] == 270 || hatDirs[1] == 0 && hatDirs[0] == 270)
      return 315;
    else
      return (hatDirs[0] + hatDirs[1]) / 2;
  }
  return JOYSTICK_HATSWITCH_RELEASE;
}

void mapButtonToJoy(int changedBtn, int pressed)
{
  const joyAction &act = btn2joy[changedBtn];
  if (act.button)
  {
    if (pressed)
      Joystick.pressButton(act.id);
    else
      Joystick.releaseButton(act.id);
  }
  else
  {
    Joystick.setHatSwitch(act.id, getHatAngle(act.id));
  }

  // left
  // 7  -> 0
  // 6 -> 90
  // 5 -> 180
  // 4 -> 270
}

void loop()
{

  for (int group = 0; group < 2; ++group)
  {
    for (int j = 0; j < 2; ++j)
    {
      int jPin = j + KEY_PORT_BEGIN[group] + KEY_ROW_OFFSET[group];

      pinMode(jPin, OUTPUT);
      digitalWrite(jPin, LOW);

      for (int i = 0; i < KEY_PORT_COUNT - 2; ++i)
      {
        int iPin = i + KEY_PORT_BEGIN[group] + KEY_COLUMN_OFFSET[group];

        pinMode(iPin, INPUT_PULLUP);
        int value = digitalRead(iPin);

        int btnIdx = j * 4 + i + KEY_ARRAY_OFFSET[group];
        int &btnValue = buttons[btnIdx];
        int btnNewValue = constrain(btnValue + (value == 0 ? 1 : -1), 0, MAX_DEBOUNCE);
        if (btnNewValue != btnValue)
        {
          btnValue = btnNewValue;

          if (btnNewValue == 0)
          {
            Serial.print(btnIdx);
            Serial.println(" released!");
            mapButtonToJoy(btnIdx, 0);
          }

          if (btnNewValue == MAX_DEBOUNCE)
          {
            Serial.print(btnIdx);
            Serial.println(" pressed!");
            mapButtonToJoy(btnIdx, 1);
          }
        }
      }

      digitalWrite(jPin, HIGH);
      pinMode(jPin, INPUT_PULLUP);
    }
  }

  // for (int i = 0; i < 16; ++i)
  // {
  //   Serial.print(buttons[i]);
  //   Serial.print(" ");
  // }

  // Serial.println();

  delay(2);
}
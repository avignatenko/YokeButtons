#include <Arduino.h>
#include <Joystick.h>

Joystick_ Joystick(JOYSTICK_DEFAULT_REPORT_ID,
                   JOYSTICK_TYPE_GAMEPAD, 19,
                   JOYSTICK_DEFAULT_HATSWITCH_COUNT,
                   false, false, false, false, false, false,
                   false, false, false, false, false);

const int KEY_ARRAY_OFFSET[2] = {0, 8};
const int KEY_PORT_BEGIN[2] = {2, A0};
const int KEY_ROW_OFFSET[2] = {0, 4};
const int KEY_COLUMN_OFFSET[2] = {2, 0};

const int KEY_PORT_COUNT = 6;

const int MAX_DEBOUNCE = 10;

const int SHIFT_BUTTON = 12;

int buttons[16] = {0};
bool shiftPressed = false;

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
  // Serial.begin(115200);
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

joyAction btn2joy_shift[16] = {
    {true, 0 + 5, 0},  // 0
    {true, 5 + 5, 0},  // 1
    {true, 4 + 5, 0},  // 2
    {true, 1 + 5, 0},  // 3
    {true, 6 + 5, 0},  // 4
    {true, 7 + 5, 0},  // 5
    {true, 8 + 5, 0},  // 6
    {true, 9 + 5, 0},  // 7
    {true, 10 + 5, 0}, // 8
    {true, 11 + 5, 0}, // 9
    {true, 12 + 5, 0}, // 10
    {true, 13 + 5, 0}, // 11
    {true, 0, 0},      // 12 // 12 is SHIFT_BUTTON!
    {true, 2 + 5, 0},  // 13
    {true, 3 + 5, 0},  // 14
    {true, 0 + 5, 0}   // 15
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
  if (changedBtn == SHIFT_BUTTON)
    shiftPressed = pressed;

  const joyAction &act = (shiftPressed ? btn2joy_shift[changedBtn] : btn2joy[changedBtn]);
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
            //Serial.print(btnIdx);
            //Serial.println(" released!");
            mapButtonToJoy(btnIdx, 0);
          }

          if (btnNewValue == MAX_DEBOUNCE)
          {
            //Serial.print(btnIdx);
            //Serial.println(" pressed!");
            mapButtonToJoy(btnIdx, 1);
          }
        }
      }

      digitalWrite(jPin, HIGH);
      pinMode(jPin, INPUT_PULLUP);
    }
  }
  delay(2);
}
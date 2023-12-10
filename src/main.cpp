#include <Arduino.h>

// ************************** НАСТРОЙКИ ***********************
#define CURRENT_LIMIT 0  // лимит по току в миллиамперах, автоматически управляет яркостью (пожалей свой блок питания!) 0 - выключить лимит
#define AUTOPLAY_TIME 30 // время между сменой режимов в секундах

#define NUM_LEDS 240     // количсетво светодиодов в одном отрезке ленты
#define LED_PIN D6       // пин ленты
#define BTN_PIN D7       // пин кнопки/сенсора
#define MIN_BRIGHTNESS 5 // минимальная яркость при ручной настройке
#define BRIGHTNESS 250   // начальная яркость
#define TRACK_STEP 50
#define MODES_AMOUNT 6 // числоэфектов

//----------------------------
#define FOR_i(from, to) for (int i = (from); i < (to); i++)
#define FOR_j(from, to) for (int j = (from); j < (to); j++)
// настройки пламени
#define HUE_GAP 21     // заброс по hue
#define FIRE_STEP 40   // шаг огня
#define HUE_START 20   // начальный цвет огня (0 красный, 80 зелёный, 140 молния, 190 розовый)
#define MIN_BRIGHT 5   // мин. яркость огня
#define MAX_BRIGHT 255 // макс. яркость огня
#define MIN_SAT 245    // мин. насыщенность
#define MAX_SAT 255    // макс. насыщенность
int counter = 0;
//----------------------------
#include "GyverButton.h"
GButton touch(BTN_PIN, LOW_PULL, NORM_OPEN);

#include <FastLED.h>
CRGB leds[NUM_LEDS];
CRGBPalette16 gPal;

#include "GyverTimer.h"
GTimer_ms effectTimer(60);
GTimer_ms autoplayTimer((long)AUTOPLAY_TIME * 1000);
GTimer_ms brightTimer(20);

int brightness = BRIGHTNESS;
int tempBrightness;
byte thisMode;

bool gReverseDirection = false;
boolean loadingFlag = true;
boolean autoplay = false;
boolean powerDirection = true;
boolean powerActive = false;
boolean powerState = true;
boolean whiteMode = false;
boolean brightDirection = true;
boolean wasStep = false;

void nextMode();
void brightnessTick();
void fillAll(CRGB newcolor);
uint32_t getPixColor(int thisPixel);
void lightBugs();
void colors();
void rainbow();
void sparkles();
void fire();
void Fire2012WithPalette();
void Fire2012WithPalette1();
void fade();
void fireTick();
void fokus();
void ciklon();

void setup()
{
  Serial.begin(9600);
  FastLED.addLeds<WS2811, LED_PIN, GRB>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  if (CURRENT_LIMIT > 0)
    FastLED.setMaxPowerInVoltsAndMilliamps(5, CURRENT_LIMIT);
  FastLED.setBrightness(brightness);
  FastLED.show();

  randomSeed(analogRead(0));
  touch.setTimeout(300);
  touch.setStepTimeout(50);
  pinMode(D4, OUTPUT);
}

void loop()
{
  touch.tick();
  if (touch.hasClicks())
  {
    byte clicks = touch.getClicks();
    switch (clicks)
    {
    case 1:
      powerDirection = !powerDirection;
      powerActive = true;
      tempBrightness = brightness * !powerDirection;
      break;
    case 2:
      if (!whiteMode && !powerActive)
      {
        nextMode();
      }
      break;

    case 3:
      if (!whiteMode && !powerActive)
        autoplay = !autoplay;
        digitalWrite(D4,!autoplay);
      break;
    default:
      break;
    }
  }

  if (touch.isStep())
  {
    if (!powerActive)
    {
      wasStep = true;
      if (brightDirection)
      {
        brightness += 5;
      }
      else
      {
        brightness -= 5;
      }
      brightness = constrain(brightness, MIN_BRIGHTNESS, 255);
      FastLED.setBrightness(brightness);
      FastLED.show();
    }
  }

  if (touch.isRelease())
  {
    if (wasStep)
    {
      wasStep = false;
      brightDirection = !brightDirection;
    }
  }

  if (effectTimer.isReady() && powerState)
  {
    switch (thisMode)
    {

    case 0:
      fireTick();
      break;
    // case 1:
    //   lightBugs(); // &&
    //   break;
    case 1:
      colors(); // цвета
      break;
    case 2:
      rainbow(); // радуга
      break;
    case 3:
      sparkles();
      break;
    case 4:
      fokus();
      break;
    case 5:
      ciklon();
      break;
    }
    FastLED.show();
  }

  if (autoplayTimer.isReady() && autoplay)
  { // таймер смены режима
    nextMode();
  }

  brightnessTick();
}

void nextMode()
{
  thisMode++;
  if (thisMode >= MODES_AMOUNT)
    thisMode = 0;
  loadingFlag = true;
  FastLED.clear();
}

void brightnessTick()
{
  if (powerActive)
  {
    if (brightTimer.isReady())
    {
      if (powerDirection)
      {
        powerState = true;
        tempBrightness += 10;
        if (tempBrightness > brightness)
        {
          tempBrightness = brightness;
          powerActive = false;
        }
        FastLED.setBrightness(tempBrightness);
        FastLED.show();
      }
      else
      {
        tempBrightness -= 10;
        if (tempBrightness < 0)
        {
          tempBrightness = 0;
          powerActive = false;
          powerState = false;
        }
        FastLED.setBrightness(tempBrightness);
        FastLED.show();
      }
    }
  }
}

// залить все
void fillAll(CRGB newcolor)
{
  for (int i = 0; i < NUM_LEDS; i++)
  {
    leds[i] = newcolor;
  }
}

// функция получения цвета пикселя по его номеру
uint32_t getPixColor(int thisPixel)
{
  return (((uint32_t)leds[thisPixel].r << 16) | ((long)leds[thisPixel].g << 8) | (long)leds[thisPixel].b);
}

// ****************************** ЦВЕТА ******************************
byte hue;
void colors()
{
  hue += 2;
  CRGB thisColor = CHSV(hue, 255, 255);
  fillAll(CHSV(hue, 255, 255));
  // мерцающие точки
  if (random8() < 80)
  {
    leds[random16(NUM_LEDS)] += CRGB::White;
  }
}
// ****************************** радуга с мерцанием ******************************
byte baza = 0; // изменение оттенка LED
void rainbow()
{
  fill_rainbow(leds, NUM_LEDS, baza++, 7);
  if (random8() < 80)
  {
    leds[random16(NUM_LEDS)] += CRGB::White;
  }
}
// ****************************** с эффектом «фокус» ******************************
void fokus()
{
  fadeToBlackBy(leds, NUM_LEDS, 2);
  for (int i = 0; i < 8; i++)
  {
    leds[beatsin16(i + 7, 0, NUM_LEDS - 1)] |= CHSV(baza += 16, 200, 255);
  }
}
// ****************************** КОНФЕТТИ ******************************
void sparkles()
{
  byte thisNum = random(0, NUM_LEDS);
  if (getPixColor(thisNum) == 0)
    leds[thisNum] = CHSV(random(0, 255), 255, 255);
  fade();
}

//**************** Огонь по всей ленте  ***********************
void fireTick()
{
  int thisPos = 0, lastPos = 0;
  FOR_i(0, NUM_LEDS)
  {
    leds[i] = CHSV(
        HUE_START + map((inoise8(i * FIRE_STEP, counter)), 0, 255, 0, HUE_GAP),                   // H
        constrain(map((inoise8(i * FIRE_STEP, counter)), 0, 255, MAX_SAT, MIN_SAT), 0, 255),      // S
        constrain(map((inoise8(i * FIRE_STEP, counter)), 0, 255, MIN_BRIGHT, MAX_BRIGHT), 0, 255) // V
    );
  }
  counter += 20;
}
//**************** с эффектом «циклон»  ***********************
void ciklon()
{
  for (int i = 0; i < NUM_LEDS; i++)
  {
    leds[i].nscale8(250);
  }
  for (int i = 0; i < NUM_LEDS; i++)
  {
    leds[i] = CHSV(baza++, 255, 255);
  }
}

// ****************** СЛУЖЕБНЫЕ ФУНКЦИИ *******************
void fade()
{
  for (int i = 0; i < NUM_LEDS; i++)
  {
    if ((uint32_t)getPixColor(i) == 0)
      continue;
    leds[i].fadeToBlackBy(TRACK_STEP);
  }
}

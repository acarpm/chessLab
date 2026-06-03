#include <Arduino.h>
#include "io/hal/HallSensor.h"
#include "const/gpio_pins.h"
#include "const/board_config.h"

HallSensor::HallSensor()
{
  pinMode(HALL_PIN, INPUT);
  for (int p : {DEC_A0, DEC_A1, DEC_A2, SEL_S0, SEL_S1, SEL_S2})
    pinMode(p, OUTPUT);
}

HallSensor &HallSensor::getInstance()
{
  static HallSensor instance;
  return instance;
}

void HallSensor::select_col(int col)
{
  digitalWrite(DEC_A0, col & 1);
  digitalWrite(DEC_A1, (col >> 1) & 1);
  digitalWrite(DEC_A2, (col >> 2) & 1);
}

void HallSensor::select_row(int row)
{
  row = (BOARD_SIZE - 1) - row;
  digitalWrite(SEL_S0, row & 1);
  digitalWrite(SEL_S1, (row >> 1) & 1);
  digitalWrite(SEL_S2, (row >> 2) & 1);
}

int HallSensor::read(int row, int col)
{
  select_row(row);
  select_col(col);
  delayMicroseconds(50);

  const int N = 30, TRIM = 6;
  int samples[N];
  for (int i = 0; i < N; i++)
  {
    samples[i] = analogRead(HALL_PIN);
    delayMicroseconds(10);
  }

  // Tri par insertion
  for (int i = 1; i < N; i++)
  {
    int key = samples[i], j = i - 1;
    while (j >= 0 && samples[j] > key)
    {
      samples[j + 1] = samples[j];
      j--;
    }
    samples[j + 1] = key;
  }

  int sum = 0;
  for (int i = TRIM; i < N - TRIM; i++)
    sum += samples[i];
  return sum / (N - 2 * TRIM);
}

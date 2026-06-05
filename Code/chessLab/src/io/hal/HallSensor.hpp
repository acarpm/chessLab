#pragma once

class HallSensor
{
private:
  HallSensor();
  void select_row(uint8_t row);
  void select_col(uint8_t col);

public:
  static HallSensor &getInstance();
  int read(uint8_t row, uint8_t col);
};

#define hallSensor HallSensor::getInstance()

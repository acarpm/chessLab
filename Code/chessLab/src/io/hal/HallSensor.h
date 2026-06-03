#pragma once

class HallSensor
{
private:
  HallSensor();
  void select_row(int row);
  void select_col(int col);

public:
  static HallSensor &getInstance();
  int read(int row, int col);
};

#define hallSensor HallSensor::getInstance()

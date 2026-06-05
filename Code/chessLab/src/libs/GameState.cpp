#include "libs/GameState.hpp"

GameState::GameState()
{
  mode = GAME_SETUP;
}

GameState &GameState::getInstance()
{
  static GameState instance;
  return instance;
}

GameStateMode GameState::getMode()
{
  return mode;
}

void GameState::setMode(GameStateMode newMode)
{
  mode = newMode;
}

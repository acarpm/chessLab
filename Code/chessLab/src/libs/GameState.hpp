#pragma once

enum GameStateMode
{
  GAME_SETUP,
  GAME_PLAYING
};

class GameState
{
private:
  GameStateMode mode;
  GameState();

public:
  static GameState &getInstance();
  GameStateMode getMode();
  void setMode(GameStateMode newMode);
};

#define gameState GameState::getInstance()

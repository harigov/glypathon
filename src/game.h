#pragma once

#include <iostream>
#include <vector>

#include "sdl2/sdl.h"

enum GameStatus
{
  Play,
  Exit
};

class Game
{
 public:
  Game();
  ~Game();
  GameStatus Status();
  void Tick();

 private:
  int LogSdlError(std::ostream& os, const std::string& msg);

  SDL_Texture* LoadTexture(const std::string& file, SDL_Renderer* ren);
  void RenderTexture(SDL_Texture* tex, SDL_Renderer* ren,
                     const int x, const int y, const int w, const int h);
  void RenderTexture(SDL_Texture* tex, SDL_Renderer* ren, const int x, const int y);
  void RenderTexture(SDL_Texture* tex, SDL_Renderer* ren, const int x, const int y,
                     const double angle);


  const std::string title_;
  GameStatus status_;
  long ticks_;
  SDL_Window* window_;
  SDL_Renderer* renderer_;
  SDL_Texture* ball_;
  std::vector<SDL_Texture*> bricks_;
};

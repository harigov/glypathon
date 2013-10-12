#include "game.h"

#include <exception>

#include "sdl2/sdl_image.h"

static const int WIDTH = 640;
static const int HEIGHT = 480;

using namespace std;

Game::Game()
    : title_("Demo Game Window :: Press 'q' to quit")
    , status_(GameStatus::Play)
    , ticks_(0)
{
  if (SDL_Init(SDL_INIT_EVERYTHING) == -1) {
    LogSdlError(cout, "SDL_Init");
    throw "SDL Init";
  }

  if ((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) != IMG_INIT_PNG)
  {
    LogSdlError(cout, "IMG_Init");
    throw "IMG Init";
  }

  const int windowFlags = SDL_WINDOW_SHOWN;
  window_ = SDL_CreateWindow(title_.c_str(), 100, 100, WIDTH, HEIGHT,
                             windowFlags);
  if (window_ == nullptr) {
    LogSdlError(cout, "SDL_CreateWindow");
    throw "SDL Create Window";
  }

  const int rendererFlags = SDL_RENDERER_ACCELERATED |
                            SDL_RENDERER_PRESENTVSYNC;

  renderer_ = SDL_CreateRenderer(window_, -1, rendererFlags);

  SDL_SetRenderDrawColor(renderer_, 75, 175, 185, 255);

  ball_ = LoadTexture("resources/ball.png", renderer_);
  bricks_.push_back(LoadTexture("resources/brick.png", renderer_));
}

Game::~Game()
{
  SDL_DestroyTexture(ball_);
  SDL_DestroyRenderer(renderer_);
  SDL_DestroyWindow(window_);
  SDL_Quit();
}

void Game::Tick()
{
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    if (event.type == SDL_KEYDOWN) {
      if (event.key.keysym.sym == 'q') {
        status_ = GameStatus::Exit;
        return;
      }
    }
  }

  SDL_RenderClear(renderer_);

  for (auto brick : bricks_)
  {
    RenderTexture(brick, renderer_, 400, 300, ticks_);
  }

  SDL_RenderPresent(renderer_);
  ++ticks_;
}

GameStatus Game::Status()
{
  return status_;
}

int Game::LogSdlError(ostream& os, const string& msg)
{
  os << msg << " error: " << SDL_GetError() << endl;
  return 1;
}

SDL_Texture* Game::LoadTexture(const string& file, SDL_Renderer* ren)
{
  SDL_Texture* tex = IMG_LoadTexture(ren, file.c_str());
  if (tex == nullptr) {
    LogSdlError(cout, "LoadTexture");
  }

  return tex;
}

void Game::RenderTexture(SDL_Texture* tex, SDL_Renderer* ren,
                         const int x, const int y, const int w, const int h)
{
  SDL_Rect dst;
  dst.x = x;
  dst.y = y;
  dst.w = w;
  dst.h = h;
  SDL_RenderCopy(ren, tex, NULL, &dst);
}

void Game::RenderTexture(SDL_Texture* tex, SDL_Renderer* ren, int x, int y)
{
  int w, h;
  SDL_QueryTexture(tex, NULL, NULL, &w, &h);
  RenderTexture(tex, ren, x, y, w, h);
}

void Game::RenderTexture(SDL_Texture* tex, SDL_Renderer* ren, int x, int y,
                         double angle)
{
  int w, h;
  SDL_QueryTexture(tex, NULL, NULL, &w, &h);
  SDL_Rect dst;
  dst.x = x;
  dst.y = y;
  dst.w = w;
  dst.h = h;
  SDL_RenderCopyEx(ren, tex, NULL, &dst, angle, NULL, SDL_FLIP_NONE);
}



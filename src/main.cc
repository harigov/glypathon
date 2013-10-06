#include <iostream>

#include "opencv2/opencv.hpp"
#include "sdl2/sdl.h"
#include "sdl2/sdl_image.h"

#include "configuration.h"
#include "glyph_detector.h"
#include "glyph_validator.h"

using namespace cv;
using namespace std;

static const int WIDTH = 640;
static const int HEIGHT = 480;

int logSdlError(ostream& os, const string& msg)
{
  os << msg << " error: " << SDL_GetError() << endl;
  return 1;
}

SDL_Texture* loadTexture(const string& file, SDL_Renderer* ren)
{
  SDL_Texture* tex = IMG_LoadTexture(ren, file.c_str());
  if (tex == nullptr) {
    logSdlError(cout, "LoadTexture");
  }

  return tex;
}

void renderTexture(SDL_Texture* tex, SDL_Renderer* ren,
                   int x, int y, int w, int h)
{
  SDL_Rect dst;
  dst.x = x;
  dst.y = y;
  dst.w = w;
  dst.h = h;
  SDL_RenderCopy(ren, tex, NULL, &dst);
}

void renderTexture(SDL_Texture* tex, SDL_Renderer* ren, int x, int y)
{
  int w, h;
  SDL_QueryTexture(tex, NULL, NULL, &w, &h);
  renderTexture(tex, ren, x, y, w, h);
}

int main()
{
  Configuration::Instance().Load("configuration.txt");

  if (SDL_Init(SDL_INIT_EVERYTHING) == -1) {
    return logSdlError(cout, "SDL_Init");
  }

  if ((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) != IMG_INIT_PNG)
  {
    return logSdlError(cout, "IMG_Init");
  }

  SDL_Window* win = SDL_CreateWindow("Non-Bouncing Ball",
                                     100, 100, WIDTH, HEIGHT,
                                     SDL_WINDOW_SHOWN);
  if (win == nullptr) {
    return logSdlError(cout, "SDL_CreateWindow");
  }

  SDL_Renderer* ren =
      SDL_CreateRenderer(win, -1,
                         SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  SDL_SetRenderDrawColor(ren, 75, 175, 185, 200);

  SDL_Texture* ball = loadTexture("resources/ball.png", ren);

  VideoCapture vc(CV_CAP_ANY);
  if (!vc.isOpened()) {
    cout << "Unable to open camera, exiting..." << endl;
    return 2;
  }

  GlyphDetector detector;

  bool quit = false;
  int x = 0, y = 300;
  SDL_Event e;
  while (!quit)
  {
    while (SDL_PollEvent(&e)) {
      if (e.type == SDL_KEYDOWN) {
        if (e.key.keysym.sym == 'q') {
          quit = true;
        }
      }
    }

    Mat frame;
    vc >> frame;

    const float factor =
      Configuration::Instance().ReadFloat("frame_resize_factor");

    if (factor != 1.0f) {
      resize(frame, frame, Size(frame.cols * factor, frame.rows * factor));
    }

    Mat gray;
    cvtColor(frame, gray, CV_BGR2GRAY);

    detector.DetectGlyph(gray);

    if(Configuration::Instance().ReadBool("display_input_frame")) {
      namedWindow("input");
      moveWindow("input", 0, 0);
      imshow("input", frame);
    }

    x = (x + 5) % WIDTH;
    y = (y + 2) % HEIGHT;
    SDL_RenderClear(ren);
    renderTexture(ball, ren, x, y);
    SDL_RenderPresent(ren);
    waitKey(1);
  }

  Configuration::Instance().Stop();
  SDL_DestroyTexture(ball);
  SDL_DestroyRenderer(ren);
  SDL_DestroyWindow(win);
  SDL_Quit();
  return 0;
}

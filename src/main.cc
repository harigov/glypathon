#include <iostream>

#include "opencv2/opencv.hpp"

#include "configuration.h"
#include "game.h"
#include "glyph.h"
#include "glyph_detector.h"

using namespace cv;
using namespace std;

int main()
{
  Configuration::Instance().Load("configuration.txt");

  GlyphDetector detector;
  Game game;

  vector<Glyph> glyphs;
  while (game.Status() != GameStatus::Exit)
  {
    if (detector.GetGlyphs(&glyphs)) {
      // TODO: Update the bricks of the game.
    }

    game.Tick();
  }

  detector.Stop();
  Configuration::Instance().Stop();
  return 0;
}

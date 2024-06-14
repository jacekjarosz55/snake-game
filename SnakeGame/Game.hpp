#pragma once
#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/bitmap.h>
#include <allegro5/events.h>
#include <allegro5/timer.h>
#include <allegro5/allegro_image.h>
#include <vector>

#include "Snake.hpp"
#include "Spritesheet.hpp"

class Game {
private:

  const unsigned WINDOW_SCALE = 1;
  const unsigned TILE_SIZE = 32;
  const unsigned TILES_X = 20;
  const unsigned TILES_Y = 20;
  const unsigned BUFFER_W = TILE_SIZE * TILES_X;
  const unsigned BUFFER_H = TILE_SIZE * TILES_Y;

  ALLEGRO_TIMER *timer;
  ALLEGRO_EVENT_QUEUE *eventQueue;
  ALLEGRO_DISPLAY *display;
  ALLEGRO_FONT *font;

  ALLEGRO_BITMAP *gameBuffer;

  Spritesheet *spritesheet;

  unsigned frameCounter = 0;

  Snake *snake;

  bool needsRedraw = false;
  bool exit = false;

  std::vector<Position> fruits;
  
  void update();
  void onKeyDown(ALLEGRO_KEYBOARD_EVENT event);
  void spawnFruit();
  void draw();
  void drawSnake();
  void drawFruits();
public:
  Game();
  ~Game();
};


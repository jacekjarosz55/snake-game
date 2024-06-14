#include <allegro5/allegro5.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_image.h>
#include <allegro5/bitmap.h>
#include <allegro5/bitmap_draw.h>
#include <allegro5/bitmap_io.h>
#include <cstdlib>
#include <iostream>

#include "Game.hpp"
#include "InitializationException.hpp"
#include "Util.hpp"

Game::Game() {
  mustInit(al_init(), "allegro");
  mustInit(al_install_keyboard(), "keyboard module");
  mustInit(al_init_image_addon(), "image addon");

  timer = al_create_timer(1.0 / 30.0); 
  mustInit(timer, "timer");

  eventQueue = al_create_event_queue();
  mustInit(eventQueue, "event queue");
  
  gameBuffer = al_create_bitmap(BUFFER_W, BUFFER_H);

  display = al_create_display(BUFFER_W*WINDOW_SCALE, BUFFER_H*WINDOW_SCALE);
  mustInit(display, "display");

  font = al_create_builtin_font();
  mustInit(font, "font");

  spritesheet = new Spritesheet("spritesheet.png", 32);

  // listen to keyboard events
  al_register_event_source(eventQueue, al_get_keyboard_event_source());
  al_register_event_source(eventQueue, al_get_display_event_source(display));
  al_register_event_source(eventQueue, al_get_timer_event_source(timer));



  // initialize game;

  snake = new Snake(0,0,5, SNAKE_DOWN);
  spawnFruit();

  // game loop
  // could be split into a separate method
  ALLEGRO_EVENT event;
  al_start_timer(timer);
  needsRedraw = false; 
  exit = false; 
  while (true) {
    al_wait_for_event(eventQueue, &event);

    switch(event.type) {
      case ALLEGRO_EVENT_KEY_DOWN:
        onKeyDown(event.keyboard);
        break;
      case ALLEGRO_EVENT_DISPLAY_CLOSE:
        exit = true;
        break;
      case ALLEGRO_EVENT_TIMER:
        update();
        needsRedraw = true;
        break;
    }

    if(exit) {
      break;
    }

    if (needsRedraw && al_is_event_queue_empty(eventQueue)) {
      draw();
      needsRedraw = false;
    }

  }
}

void Game::update() {
  frameCounter++;
  // TODO: replace '4' with snake speed
  if (frameCounter % 4 == 0) {

    snake->step();
    auto head = snake->getHead();
    if (snake->hasCollidedWithSelf() || 
      head.x < 0 || head.x >= TILES_X || head.y < 0 || head.y >= TILES_Y) {
      exit = true;
    }


    for (int i = 0; i < fruits.size(); i++) {
      if(snake->hasCollidedWith(fruits[i])) {
        fruits.erase(fruits.begin() + i);
        snake->addLength(1);
        spawnFruit();
        break;
      }
    }

  }
}

// kinda weird but this is the only input we need so
void Game::onKeyDown(ALLEGRO_KEYBOARD_EVENT event) {
  if (event.keycode == ALLEGRO_KEY_ESCAPE) {
    exit = true;
  }

  if (event.keycode == ALLEGRO_KEY_LEFT) {
    snake->turn(SNAKE_LEFT);
  }
  if (event.keycode == ALLEGRO_KEY_RIGHT) {
    snake->turn(SNAKE_RIGHT);
  }
  if (event.keycode == ALLEGRO_KEY_UP) {
    snake->turn(SNAKE_UP);
  }
  if (event.keycode == ALLEGRO_KEY_DOWN) {
    snake->turn(SNAKE_DOWN);
  }
}



void Game::draw() {
  // draw on the game buffer
  al_set_target_bitmap(gameBuffer); 
  al_clear_to_color(al_map_rgb(0,0,0));

  drawFruits();
  drawSnake();
  
  // draw the buffer onto the window
  al_set_target_backbuffer(display); 
  al_draw_scaled_bitmap(gameBuffer, 0, 0, BUFFER_W, BUFFER_H, 0, 0, BUFFER_W * WINDOW_SCALE, BUFFER_H * WINDOW_SCALE, 0);
  al_flip_display();
}


void Game::drawSnake() {
  const unsigned SNAKE_HEAD_SPRITE = 0;
  const unsigned SNAKE_BODY_SPRITE = 1;
  const unsigned SNAKE_TURN_SPRITE = 2;
  const unsigned SNAKE_TAIL_SPRITE = 6;
  const float PI = 3.14;
  float halfsize = (float)TILE_SIZE / 2;
  float angle = 0;


  // draw head
  auto snakeBody = snake->getBody();
  auto head = snakeBody.back();
  switch(snake->getDirection()) {
  case SNAKE_LEFT:
    angle = 0 * PI;
    break;
  case SNAKE_RIGHT:
    angle = 1 * PI;
    break;
  case SNAKE_UP:
    angle = 0.5 * PI;
    break;
  case SNAKE_DOWN:
    angle = 1.5 * PI;
    break;
  }

  al_draw_rotated_bitmap(
    spritesheet->get(0),
    halfsize, halfsize,
    head.x*TILE_SIZE + halfsize,
    head.y*TILE_SIZE + halfsize,
    angle,
    0);

  if (snakeBody.size() <= 1) return;

  // draw tail
  int dx, dy = 0;
  Position tail = snakeBody[0];
  Position tailNext = snakeBody[1];
  dx = tailNext.x - tail.x;
  dy = tailNext.y - tail.y;
  if (dx > 0)  { angle = PI * 0.5; }
  if (dx < 0)  { angle = PI * 1.5; }
  if (dy > 0)  { angle = PI; }
  if (dy < 0)  { angle = 0 * PI;  }
  

  al_draw_rotated_bitmap(
    spritesheet->get(SNAKE_TAIL_SPRITE),
    halfsize, halfsize,
    tail.x*TILE_SIZE + halfsize,
    tail.y*TILE_SIZE + halfsize,
    angle,
    0);

  // draw the rest
  for (int i = 1; i < snakeBody.size() - 1; i++) {
    // TODO: check previous and next element to determine which tile to draw
    Position prevPart = snakeBody[i-1];
    Position part = snakeBody[i];
    Position nextPart = snakeBody[i+1];

    bool isTurn = !(prevPart.x == nextPart.x || prevPart.y == nextPart.y);
    angle = 0;
    if (!isTurn && nextPart.y != prevPart.y) {
        angle = 0.5 * PI;
    }
    al_draw_rotated_bitmap(
      spritesheet->get(isTurn ? SNAKE_TURN_SPRITE : SNAKE_BODY_SPRITE),
      halfsize,
      halfsize,
      part.x*TILE_SIZE+halfsize,
      part.y*TILE_SIZE+halfsize,
      angle,
      0);
  }
}

void Game::drawFruits() {
  for (auto fruit : fruits) {
    al_draw_bitmap(spritesheet->get(4), fruit.x*TILE_SIZE, fruit.y*TILE_SIZE, 0);
  }
}

void Game::spawnFruit() {
  Position fruit;
  do {
    fruit.x = rand()%TILES_X;
    fruit.y = rand()%TILES_Y;
  } while (snake->isInside(fruit));
  fruits.push_back(fruit);
}


Game::~Game() {
  delete spritesheet;
  delete snake;
  al_destroy_font(font);
  al_destroy_bitmap(gameBuffer);
  al_destroy_display(display);
  al_destroy_timer(timer);
  al_destroy_event_queue(eventQueue);
}

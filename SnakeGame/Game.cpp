#include <allegro5/allegro5.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_image.h>
#include <allegro5/bitmap.h>
#include <allegro5/bitmap_draw.h>
#include <allegro5/bitmap_io.h>
#include <allegro5/allegro_audio.h>
#include<allegro5/allegro_acodec.h>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <fstream>

#include "Game.hpp"
#include "InitializationException.hpp"
#include "Util.hpp"


Game::Game() {
  mustInit(al_init(), "allegro");
  mustInit(al_install_keyboard(), "keyboard module");
  mustInit(al_init_image_addon(), "image addon");
  mustInit(al_init_font_addon(), "font addon");
  mustInit(al_init_ttf_addon(), "ttf addon");

  mustInit(al_install_audio(), "audio");
  mustInit(al_init_acodec_addon(), "audio codecs");
  mustInit(al_reserve_samples(16), "reserve samples");

  pickupSound = al_load_sample("pickup.wav");
  mustInit(pickupSound, "pickup sound");
  gameOverSound = al_load_sample("gameOver.wav");
  mustInit(gameOverSound, "game over sound");

  timer = al_create_timer(1.0 / 30.0); 
  mustInit(timer, "timer");

  eventQueue = al_create_event_queue();
  mustInit(eventQueue, "event queue");
  
  gameBuffer = al_create_bitmap(BUFFER_W, BUFFER_H);

  

  display = al_create_display(BUFFER_W*WINDOW_SCALE, BUFFER_H*WINDOW_SCALE);
  mustInit(display, "display");

  font = al_load_ttf_font("Roboto-Bold.ttf", fontSize, 0);
  mustInit(font, "font");

  spritesheet = new Spritesheet("spritesheet.png", 32);

  // listen to keyboard events
  al_register_event_source(eventQueue, al_get_keyboard_event_source());
  al_register_event_source(eventQueue, al_get_display_event_source(display));
  al_register_event_source(eventQueue, al_get_timer_event_source(timer));


  initMenu();
  
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
          switch (state) {
          case GAME_MENU:
              onKeyDownMenu(event.keyboard);
              break;
          case GAME_PLAYING:
              onKeyDownGame(event.keyboard);
              break;
          }
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

bool Game::collidesWithMap(Position pos) {
    for (Position& tile : mapData) {
        if (pos.x == tile.x && pos.y == tile.y) return true;
    }
    return false;
}


void Game::drawMap() {


    for (Position &tile : mapData) {
        al_draw_bitmap(spritesheet->get(9), tile.x*TILE_SIZE, tile.y*TILE_SIZE, 0);
    }
}

void Game::loadMap() {
    std::ifstream mapfile;
    std::stringstream filename;
    filename << "map" << mapIndex << ".txt";
    mapfile.open(filename.str());

    if (!mapfile.is_open()) {
        throw new InitializationException(filename.str());
    }
    mapData.clear();
    std::string line;
    Position currentPos{};
    while (std::getline(mapfile, line)) {
        for (char x : line) {
            if (x == '0') {
                currentPos.x += 1;
            }
            if (x == '1') {
                mapData.push_back(currentPos);
                currentPos.x += 1;
            }
        }
        currentPos.y += 1;
        currentPos.x = 0;
    }
    mapfile.close();
}

void Game::initGame() {
    state = GAME_PLAYING;
    if (snake) {
        delete snake;
    }
    snake = new Snake(4, 4, 2, SNAKE_RIGHT);
    score = 0;
    loadMap();
    fruits.clear();
    spawnFruit();
}

void Game::update() {
    switch (state) {
    case GAME_MENU:
        updateMenu();
        break;
    case GAME_PLAYING:
        updateGame();
        break;
    }
}
void Game::gameOver() {
    if (hardMode) score *= 2;
    al_play_sample(gameOverSound, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
    initMenu();
}
void Game::updateMenu() {

}

void Game::updateGame() {
  frameCounter++;
  if (frameCounter % (hardMode ? 3 : 4) == 0) {
    snake->step();
    score += 1;
    auto head = snake->getHead();
    if (snake->hasCollidedWithSelf() || collidesWithMap(head) || 
      head.x < 0 || head.x >= TILES_X || head.y < 0 || head.y >= TILES_Y) {
        // game over
        gameOver();
    }


    for (int i = 0; i < fruits.size(); i++) {
      if(snake->hasCollidedWith(fruits[i])) {
        fruits.erase(fruits.begin() + i);
        al_play_sample(pickupSound, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
        score += 10;
        snake->addLength(1);
        spawnFruit();
        break;
      }
    }

  }
}


void Game::onKeyDownMenu(ALLEGRO_KEYBOARD_EVENT event) {
    if (event.keycode == ALLEGRO_KEY_SPACE) {
        initGame();
    }
    if (event.keycode == ALLEGRO_KEY_ESCAPE) {
        exit = true;
    }

    if (event.keycode == ALLEGRO_KEY_RIGHT) {
        mapIndex = (mapIndex + 1) % mapCount;
    }

    if (event.keycode == ALLEGRO_KEY_LEFT) {
        mapIndex = (mapIndex - 1) % mapCount;
    }

    if (event.keycode == ALLEGRO_KEY_C) {
        if (snakeColor == SNAKE_COLOR_DEFAULT) {
            snakeColor = SNAKE_COLOR_ALT;
        }
        else {
            snakeColor = SNAKE_COLOR_DEFAULT;
        }
    }

    if (event.keycode == ALLEGRO_KEY_X) {
        hardMode = !hardMode;
    }
}

void Game::initMenu() {
    state = GAME_MENU;
}

void Game::onKeyDownGame(ALLEGRO_KEYBOARD_EVENT event) {
  if (event.keycode == ALLEGRO_KEY_ESCAPE) {
      gameOver();
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


void Game::drawMenu() {
    
    std::stringstream mapText, scoreText, hardModeText, colorText;
    scoreText << "Game Over! Score: " << score;

    mapText << "Map: " << mapIndex << " (Arrow keys to select)";

    hardModeText << "Hard mode: " << (hardMode ? "Enabled (2x score)" : "Disabled") << " (X to toggle)";

    colorText << "Snake color: " << ((snakeColor == SNAKE_COLOR_DEFAULT) ? "Default" : "Alternative") << " (C to switch)";



    al_clear_to_color(al_map_rgb(0, 0, 0));
    al_draw_text(font, al_map_rgb(255, 255, 255), 10, fontSize, 0, (score >= 0) ? scoreText.str().c_str() : "Welcome to Snake!");
    al_draw_text(font, al_map_rgb(255, 255, 255), 10, fontSize*2, 0, mapText.str().c_str());
    al_draw_text(font, al_map_rgb(255, 255, 255), 10, fontSize*3, 0, colorText.str().c_str());
    al_draw_text(font, al_map_rgb(255, 255, 255), 10, fontSize*4, 0, hardModeText.str().c_str());
    al_draw_text(font, al_map_rgb(255, 255, 255), 10, fontSize*5, 0, "Space: start game");
    al_draw_text(font, al_map_rgb(255, 255, 255), 10, fontSize*6, 0, "Escape: exit game");
    al_flip_display();
}

void Game::draw() {
    switch (state) {
    case GAME_MENU:
        drawMenu();
        break;
    case GAME_PLAYING:
        drawGame();
        break;
    }
}

void Game::drawGame() { 
  // draw on the game buffer
  al_set_target_bitmap(gameBuffer); 
  al_clear_to_color(al_map_rgb(0,0,0));

  drawBackground();
  drawMap();
  drawFruits();
  drawSnake();

  // draw the buffer onto the window
  al_set_target_backbuffer(display); 
  al_draw_scaled_bitmap(gameBuffer, 0, 0, BUFFER_W, BUFFER_H, 0, 0, BUFFER_W * WINDOW_SCALE, BUFFER_H * WINDOW_SCALE, 0);
  al_flip_display();
}




void Game::drawSnake() {
    unsigned snakeHeadSprite = 0;
    unsigned snakeBodySprite = 0;
    unsigned snakeTailSprite = 0;
    switch (snakeColor) {
    case SNAKE_COLOR_DEFAULT:
        snakeHeadSprite = 0;
        snakeBodySprite = 1;
        snakeTailSprite = 2;
        break;
    case SNAKE_COLOR_ALT:
        snakeHeadSprite = 4;
        snakeBodySprite = 5;
        snakeTailSprite = 6;
        break;
    }
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
    spritesheet->get(snakeHeadSprite),
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
  if (dx > 0)  { angle = PI * 1; }
  if (dx < 0)  { angle = PI * 0; }
  if (dy > 0)  { angle = PI * 1.5; }
  if (dy < 0)  { angle = PI * 0.5; }
  
  al_draw_rotated_bitmap(
    spritesheet->get(snakeTailSprite),
    halfsize, halfsize,
    tail.x*TILE_SIZE + halfsize,
    tail.y*TILE_SIZE + halfsize,
    angle,
    0);

  for (int i = 1; i < snakeBody.size() - 1; i++) {
      Position part = snakeBody[i];
      al_draw_bitmap(spritesheet->get(snakeBodySprite), part.x * TILE_SIZE, part.y * TILE_SIZE, 0);
  }

  // draw the rest
  /*
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
      */
}

void Game::drawBackground() {
    al_hold_bitmap_drawing(true);
    for (int y = 0; y < TILES_Y; y++) {
        for (int x = 0; x < TILES_X; x++) {
            al_draw_bitmap(spritesheet->get(10), x * TILE_SIZE, y * TILE_SIZE, 0);
        }
    }
    al_hold_bitmap_drawing(false);
}

void Game::drawFruits() {
  for (auto fruit : fruits) {
    al_draw_bitmap(spritesheet->get(8), fruit.x*TILE_SIZE, fruit.y*TILE_SIZE, 0);
  }
}

void Game::spawnFruit() {
  Position fruit{};
  do {
    fruit.x = rand()%TILES_X;
    fruit.y = rand()%TILES_Y;
  } while (snake->isInside(fruit) || collidesWithMap(fruit));
  fruits.push_back(fruit);
}

Game::~Game() {
  delete spritesheet;
  delete snake;
  al_destroy_sample(gameOverSound);
  al_destroy_sample(pickupSound);
  al_destroy_font(font);
  al_destroy_bitmap(gameBuffer);
  al_destroy_display(display);
  al_destroy_timer(timer);
  al_destroy_event_queue(eventQueue);
}

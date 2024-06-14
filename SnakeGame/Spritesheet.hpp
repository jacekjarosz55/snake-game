#pragma once

#include <allegro5/bitmap.h>
#include <vector>

class Spritesheet {
private:
  unsigned _res;
  ALLEGRO_BITMAP *_source;
  unsigned _spritesX;
  unsigned _spritesY;
  unsigned _spriteCount;
  std::vector<ALLEGRO_BITMAP *> _sprites;
public:
  Spritesheet(const char *filename, unsigned resolution);
  ~Spritesheet();
  ALLEGRO_BITMAP *get(unsigned idx);
  unsigned size() const { return _sprites.size(); };
  unsigned resolution() const { return _res; };
};

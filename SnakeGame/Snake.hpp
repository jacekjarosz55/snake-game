#pragma once

#include "Util.hpp"
#include <deque>
#include <queue>


struct SnakeMove {
  int dx = 0;
  int dy = 0;
};

enum SnakeDirection {
  SNAKE_LEFT = 0,
  SNAKE_RIGHT = 1,
  SNAKE_UP = 2,
  SNAKE_DOWN = 3
};

enum SnakeColor {
	SNAKE_COLOR_DEFAULT,
	SNAKE_COLOR_ALT
};


class Snake {
private:
  unsigned _resize = 0;
  std::deque<Position> _body;
  std::queue<SnakeDirection> _moves;
  SnakeDirection _direction;

  
public: 
  Snake(int x, int y, int resize, SnakeDirection direction);
  static SnakeMove getMove(SnakeDirection direction);
  static SnakeDirection oppositeDirection(SnakeDirection direction);

  Position getHead();
  void step(); 
  bool isInside(Position pos);
  bool hasCollidedWith(Position pos);
  bool hasCollidedWithSelf();
  void addLength(unsigned len);

  SnakeColor getColor() const;
  void setColor(SnakeColor color);


  void turn(SnakeDirection direction);

  std::deque<Position> getBody() const;
  SnakeDirection getDirection() const;
};

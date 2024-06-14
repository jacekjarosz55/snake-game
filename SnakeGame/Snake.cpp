#include "Snake.hpp"
#include "Util.hpp"
#include <deque>


Snake::Snake(int x, int y, int resize, SnakeDirection direction) {
  _body.push_back({x,y});
  _resize = resize;
  _direction = direction;
}

SnakeDirection Snake::oppositeDirection(SnakeDirection direction) {
  switch (direction) {
    case SNAKE_UP:
      return SNAKE_DOWN;
    case SNAKE_DOWN:
      return SNAKE_UP;
    case SNAKE_LEFT:
      return SNAKE_RIGHT;
    case SNAKE_RIGHT:
      return SNAKE_LEFT;
  }
  return SNAKE_DOWN;
}

SnakeMove Snake::getMove(SnakeDirection direction) {
  SnakeMove move;
  move.dx=0;
  move.dy=0;

  switch (direction) {
    case SNAKE_UP:
      move.dy = -1;
      break;
    case SNAKE_DOWN:
      move.dy = 1;
      break;
    case SNAKE_LEFT:
      move.dx = -1;
      break;
    case SNAKE_RIGHT:
      move.dx = 1;
      break;
  }

  return move;
}


void Snake::turn(SnakeDirection direction) {
  if (
    ( _moves.size() == 0 && (direction == _direction || direction == oppositeDirection(_direction))
    || (_moves.size() > 0 && (_moves.front() == direction || _moves.front() == oppositeDirection(direction))))
  ) {
    return;
  }
  _moves.push(direction);
}

void Snake::step() {
  Position head = _body.back();
  Position newHead;

  if (_moves.size() > 0) {
    _direction = _moves.front();
    _moves.pop();
  }

  SnakeMove move = getMove(_direction);

  newHead.x = head.x + move.dx;
  newHead.y = head.y + move.dy;

  _body.push_back(newHead);
  if (_resize > 0) {
    _resize--;
  } else {
    _body.pop_front();
  }
}

bool Snake::hasCollidedWith(Position pos) {
  auto head = _body.back();
  return (head.x == pos.x && head.y == pos.y);
}

bool Snake::isInside(Position pos) {
  for (auto part : _body) {
    if (part.x == pos.x && part.y == pos.y) return true;
  }
  return false;
}

bool Snake::hasCollidedWithSelf() {
  Position head = _body.back();
  // exclude the head
  for (int i = 0; i < _body.size()-1; i++) {
    if (_body[i].x == head.x && _body[i].y == head.y) return true;
  }
  return false;
}

std::deque<Position> Snake::getBody() const {
  return _body;
}

Position Snake::getHead() {
  return _body.back();
}

void Snake::addLength(unsigned len) {
  _resize+=len;
}

SnakeDirection Snake::getDirection() const {
  return _direction;
}


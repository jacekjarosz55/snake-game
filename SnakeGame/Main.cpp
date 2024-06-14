#include "Game.hpp"
#include "InitializationException.hpp"
#include <iostream>

int main() {
  try {
    Game game;
    return 0;
  } 
  catch(InitializationException e) {
    std::cerr << e.what() << "\n";
    return 1;
  }
}

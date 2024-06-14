#include "Util.hpp"
#include "InitializationException.hpp"
#include <string>

void mustInit(bool check, const std::string& what) {
  if (!check) {
    throw InitializationException(what);
  }
}

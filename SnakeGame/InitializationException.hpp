#include <exception>
#include <string>
#include <sstream>

class InitializationException : public std::exception {
private:
  std::string msg;
public:
  InitializationException(const std::string &what) {
    std::stringstream msgstr;
    msgstr << "Couldn't initialize: " << what;
    msg = msgstr.str();
  }

  std::string what() {
    return msg.c_str();
  }
};


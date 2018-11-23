#ifndef __CONST_H__
#define __CONST_H__

#include <string>
#include <exception>

const bool DEBUG(false);
const bool PRINT_RULES(false);

struct StringException : public std::exception {
  StringException(std::string str_) : str(str_) {}

  char const *what() const noexcept {
    return str.c_str();
  }

  std::string str;
};


#endif

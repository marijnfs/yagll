#ifndef __STRING_EXCEPTION_H__
#define __STRING_EXCEPTION_H__

struct StringException : public std::exception {
  StringException(std::string str_) : str(str_) {}

  char const *what() const noexcept {
    return str.c_str();
  }

  std::string str;
};

#endif
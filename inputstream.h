#pragma once
#include <fstream>
#include <iostream>
#include <string>

class InputStream {
    public:
  InputStream(void) {}
  InputStream(std::string s);
  char get(void);
  char peek(void);
  char peek(int i);
  int gcount(void);
  void putback(char c);
  void add(void);
  void add(std::string s);
  bool get_full_line(void);
  bool get_file(std::string s);
  std::string spit_out(void);
  std::string expose_next(int n);

    private:
  std::string buffer;
  int index_pos{0};
  void finishFile(void);
  void reset(void);
};

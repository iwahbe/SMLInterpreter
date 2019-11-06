#include "inputstream.h"

void InputStream::add(void)
{
  for (char c; (std::cin >> std::noskipws >> c) && c != EOF;) {
    buffer.push_back(c);
  }
}
void InputStream::add(std::string s)
{
  for (int i = 0; i < s.size(); i++)
    buffer.push_back(s[i]);
}

char InputStream::get(void) { return buffer.at(index_pos++); }

char InputStream::peek(void) { return buffer.at(index_pos); }
char InputStream::peek(int i) { return buffer.at(index_pos + i); }

// InputStream::gcount: The count of unprocessed chars
//
// return int: the count

int InputStream::gcount(void) { return buffer.size() - index_pos; }

// InputStream::putback: Puts back c iff it is the correct char
//
// char c: the char to put back

void InputStream::putback(char c)
{
  if (!(c == buffer.at(index_pos - 1)))
    std::cout << "c: " << c << "buffer.at(" << index_pos - 1
              << "): " << buffer.at(index_pos - 1) << "\n";
  assert(c == buffer.at(index_pos - 1));
  index_pos--;
}

// InputStream::spit_out: Returns all unproccesed contents
//
// return std::string: a string

std::string InputStream::spit_out(void)
{
  return buffer.substr(index_pos, buffer.size());
}

// InputStream::addline: adds a single line from std::cin

bool InputStream::get_full_line(void)
{
  reset();
  for (char c; (std::cin >> std::noskipws >> c) && c != EOF && c != '\n';)
    buffer.push_back(c);
  if (buffer.size()) finishFile();
  return buffer.size();
}

void InputStream::finishFile(void)
{
  buffer.push_back(' ');
  buffer.push_back('e');
  buffer.push_back('o');
  buffer.push_back('f');
  buffer.push_back(EOF);
}

std::string InputStream::expose_next(int n)
{
  return buffer.substr(
      index_pos, std::min(n, static_cast<int>(buffer.size() - index_pos - 1)));
}

InputStream::InputStream(std::string s)
{
  buffer = s;
  finishFile();
}

void InputStream::reset(void)
{
  buffer.clear();
  index_pos = 0;
}

bool InputStream::get_file(std::string s)
{
  std::ifstream file(s);
  if (file.is_open()) {
    char c;
    while (file.get(c))
      buffer.push_back(c);
    finishFile();
    file.close();
    return true;
  }
  else {
    return false;
  }
}

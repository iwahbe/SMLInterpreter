#pragma once
#include "inputstream.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <list>
#include <vector>

using std::list;
using std::string;
using std::vector;
using token = std::string;
using std::to_string;

template <typename T> bool in_vector(vector<T> &vec, T obj);

struct Position {
  int line_number;
  int column_number;
};

class TokenStream {
    public:
  TokenStream(string sourcename, InputStream *source_)
  {
    source_name = sourcename;
    source = source_;
    analyze();
  }
  // The Tokenizer itself
  void analyze(void);
  token next(void);    // returns the token at the front, doesn't change state
  token advance(void); // pops the token at the front, returning it
  string report(void); // reports the locaion of errors in the source code
  token eat(token tk); // eats a token if it is the next token, otherwise error
  int eatInt(void);    // eats the next token if it's an integer, else error
  token eatName(void); // eats the next token if it's a name, else error
  token eatString(void); // eats next token if string, else error
  vector<string> RESERVED = {"if",     "then",    "else", "let", "val",
                             "fun",    "and",     "in",   "end", "fn",
                             "orelse", "andalso", "div",  "mod", "true",
                             "false",  "print",   "fst",  "snd", "eof"};
  bool nextIsInt(void);    // Checks if next token is an integer literal token.
  bool nextIsName(void);   // Checks if next token is a name.
  bool nextIsString(void); // Checks if next token is a string literal.

  void checkEOF(void); // Checks if the next token indicates end of file

  // Characters that separate expressions.
  string DELIMITERS = "();,|";

  // Characters that make up unary and binary operations.
  string OPERATORS = "+-*/<>=&!:.";

  // whitespace representatives
  string WHITESPACE = " \t\n\r";

  string vomit(void); // gives a string of every token without changing state

    private:
  // Variable to manage internal state
  // variables manage return state
  string source_name{""};
  InputStream *source;
  list<token> tokens;
  vector<Position> starts;
  // variable to manage parsing state
  int line{0};
  int column{1};
  Position mark;

  // Parser Helper functions
  void lexassert(char c);         // confirms that c is a valid char
  void lexassert(bool assertion); // raises a lexerror if assertion is false
  void lexassert(bool assertion, string msg);
  void raiseLex(string msg); // assembles a message then raises a lex error
  // Tokenizer helper functions
  void initIssue(void);
  void markIssue(void);
  void issue(token tk);
  char nxt(int lookahead);
  void chompSelector(void);
  void chompWord(void);
  void chompInt(void);
  void chompString(void);
  void chompComment(void);
  void chomp(void);
  char chompChar(void);
  void chompWhitespace(bool withinToken = false);
  void chompOperator(void);

  // object to manage source, allowing lookahead without loading all of source
  // into memory
};

// Lex Error Classes
class LexError : public std::exception {
    public:
  LexError(string error_msg) { msg = error_msg; }
  string what() { return msg; }

    protected:
  string msg;
};
class TypeError : public LexError {
    public:
  TypeError(string error_msg) : LexError(error_msg) {}
};
class NotImplemented : public LexError {
    public:
  NotImplemented(string error_msg) : LexError(error_msg) {}
};
class RunTimeError : public LexError {
    public:
  RunTimeError(string error_msg) : LexError(error_msg) {}
};
class ParseError : public LexError {
    public:
  ParseError(string error_msg) : LexError(error_msg) {}
};
class SyntaxError : public LexError {
    public:
  SyntaxError(string error_msg) : LexError(error_msg) {}
};

string char_string(char c);

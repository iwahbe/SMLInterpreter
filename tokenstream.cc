#include "tokenstream.h"

void TokenStream::lexassert(bool assertion)
{
  if (!assertion) throw LexError("Unexpected Occurrence");
}

void TokenStream::lexassert(bool assertion, string msg)
{
  if (!assertion) throw LexError("Unexpected Occurrence: " + msg);
}

// TokenStream::raiseLex: Assembles together an error message from text and
// internal state
//
// string msg: message about error

void TokenStream::raiseLex(string msg)
{
  string s =
      source_name + " line " + to_string(line) + " column " + to_string(column);
  s += ": " + msg;
  throw LexError(s);
}

// TokenStream::next: non-consuming next token
//
// return string: token

token TokenStream::next(void) { return tokens.front(); }

token TokenStream::advance(void)
{
  token temp = next();
  if (tokens.size() > 0) tokens.pop_front();
  if (starts.size() > 0) starts.pop_back();
  return temp;
}

string TokenStream::report(void)
{
  string s;
  s += source_name;
  s += " line " + to_string(starts.back().line_number);
  s += " column " + to_string(starts.back().column_number);
  return s;
}

token TokenStream::eat(token tk)
{
  if (tk == next()) {
    return advance();
  }
  else {
    string err = "Unexpected token at " + report() + ". ";
    err += "Saw: '" + next() + "'. ";
    err += "Expected: '" + tk + "'. ";
    throw SyntaxError{err};
  }
}

int TokenStream::eatInt(void)
{
  if (nextIsInt()) {
    // This should handle converting positive and negative ints
    string tmp = advance();
    int stoi = std::stoi(tmp);
    return stoi;
  }
  else {
    string err = "Unexpected token at " + report() + ". ";
    err += "Saw: '" + next() + "'. ";
    err += "Expected an integer literal. ";
    throw SyntaxError{err};
  }
}

bool TokenStream::nextIsInt(void)
{
  token tk{next()};
  for (int i = 0; i < tk.size(); i++)
    if (!isdigit(tk[i])) return false;
  return tk.size() > 0;
}

token TokenStream::eatName(void)
{
  if (nextIsName())
    return advance();
  else {
    string err = "Unexpected token at " + report() + ". ";
    err += "Saw: '" + next() + "'. ";
    err += "Expected a name. ";
    throw SyntaxError{err};
  }
}

bool TokenStream::nextIsName(void)
{
  token tk{next()};
  bool isname{isalpha(tk[0]) || tk[0] == '_'};
  for (int i{1}; i < tk.size(); i++)
    isname = isname && (isalnum(tk[i]) || tk[i] == '_');
  return tk.size() > 0 && isname && !in_vector<string>(RESERVED, tk);
}

token TokenStream::eatString(void)
{
  if (nextIsString()) {
    token tk{advance()};
    return tk.substr(1, tk.size() - 2);
  }
  else {
    string err = "Unexpected token at " + report() + ". ";
    err += "Saw: '" + next() + "'. ";
    err += "Expected a string literal. ";
    throw SyntaxError{err};
  }
}

// TokenStream::nextIsString: Checks if the next token is a string
//
// return bool: bool

bool TokenStream::nextIsString(void)
{
  token tk{next()};
  return tk[0] == '"' && tk[tk.size() - 1] == '"';
}

// TokenStream::initIssue: Initializes issue

void TokenStream::initIssue(void) { markIssue(); }

void TokenStream::markIssue(void) { mark = Position{line, column}; }

// TokenStream::issue: Issues a new token to tokens
//
// token tk: the token to issue

void TokenStream::issue(token tk)
{
  tokens.push_back(tk);
  starts.push_back(mark);
  markIssue();
}

// TokenStream::nxt: Returns the char at lookahead
//
// int lookahead: how far ahead to look
//
// return char: char in source

char TokenStream::nxt(int lookahead = 0) { return source->peek(lookahead); }

void TokenStream::chompSelector(void)
{
  lexassert(nxt() == '#' || isdigit(nxt(2)), "chompSelector");
  chompChar();
  string tk = "#";
  while (isdigit(nxt()))
    tk += chompChar();
  issue(tk);
}

// TokenStream::chompWord: Eats a word into a token

void TokenStream::chompWord(void)
{
  lexassert(isalnum(nxt()) || (nxt() == '_'), "chompWord");
  string tk;
  tk += chompChar();
  while (isalnum(nxt()))
    tk += chompChar();
  issue(tk);
}

// TokenStream::chompInt: Eats an int into a token

void TokenStream::chompInt(void)
{
  // Asserts that nxt() is a digit
  lexassert(isdigit(nxt()), "chompInt");
  string tk = {""};
  tk += chompChar();
  while (isdigit(nxt()))
    tk += chompChar();
  lexassert(tk != "", "chompInt generated an empty token");
  issue(tk);
}

// TokenStream::chompString: Eats a string into a token

void TokenStream::chompString(void)
{
  lexassert(nxt() == '"', "chompString");
  chompChar(); // East the quote
  string tk = {""};
  while (nxt() != '"') {
    if (nxt() == '\\') {
      chompChar();
      if (nxt() == '\n')
        chompWhitespace(true);
      else if (nxt() == '\\')
        tk += chompChar();
      else if (nxt() == 'n') {
        chompChar();
        tk += '\n';
      }
      else if (nxt() == 't') {
        chompChar();
        tk += '\t';
      }
      else if (nxt() == '"') {
        chompChar();
        tk += '"';
      }
      else
        raiseLex("Bad string escape character");
    }
    else if (nxt() == '\n')
      raiseLex("End of line encountered within string");
    else if (nxt() == '\t')
      raiseLex("Tab encountered within string");
    else
      tk += chompChar();
  }
  if (nxt() == EOF)
    raiseLex("EOF encountered within string");
  else {
    chompChar();
    issue('"' + tk + '"');
  }
}

// TokenStream::chompComment: Eats a comment, no token issued

void TokenStream::chompComment(void)
{
  lexassert(source->gcount() > 1, "chompComment");
  // confirming that this is a comment
  char c;
  lexassert((c = chompChar()) == '(', "chompComment");
  lexassert((c = chompChar()) == '*', "chompComment");
  while (source->gcount() >= 2 && !(source->expose_next(2) == "*)"))
    chompChar();
  if (source->gcount() < 2)
    raiseLex("EOF encountered within comment");
  else {
    // must be end of comment
    chompChar();
    chompChar();
  }
}

// TokenStream::chomp: Eats either whitespace or a char

void TokenStream::chomp(void)
{
  if (nxt() == '\n' || nxt() == '\t' || nxt() == '\r')
    chompWhitespace();
  else
    chompChar();
}

// TokenStream::chompChar: This eats a char from the stream

char TokenStream::chompChar(void)
{
  lexassert(source->peek() != EOF, "chompChar");
  char c = source->get();
  column++;
  return c;
}

// TokenStream::chompWhitespace: Eats whitespace
//
// bool withintoken: currently within a token

void TokenStream::chompWhitespace(bool withintoken)
{
  lexassert(source->peek() != EOF, "chompWhitespace");
  char c = source->get();
  if (c == ' ')
    column++;
  else if (c == '\t')
    column += 4;
  else if (c == '\n') {
    line++;
    column = 0;
  }
  if (!withintoken) markIssue();
}

// TokenStream::chompOperator: Eats an operator and issues a token

void TokenStream::chompOperator(void)
{
  string tk;
  while (OPERATORS.find(nxt()) != -1)
    tk += chompChar();
  issue(tk);
}

// TokenStream::analyze: Consumes the InputStream object 'source', and creates
// tokens

void TokenStream::analyze(void)
{
  while (source->gcount() > 1) { // due to the EOF at the end
    // CHOMP a string literal
    if (source->peek() == '"') chompString();
    // CHOMP a comment
    else if (nxt() == '(' && nxt(1) == '*')
      chompComment();
    // CHOMP whitespace
    else if (WHITESPACE.find(nxt()) != -1)
      chompWhitespace();
    // CHOMP an integer literal
    else if (isdigit(nxt()))
      chompInt();
    // CHOMP a single "delimiter" char
    else if (DELIMITERS.find(nxt()) != -1)
      issue(char_string(chompChar()));
    // CHOMP an operator
    else if (OPERATORS.find(nxt()) != -1)
      chompOperator();
    // CHOMP a reserved word or name
    else
      chompWord();
  }
}

// in_vector: Tests if an object is in a vector
//
// vector<T> vec: Vector type
// T obj: obj to test
//
// return template <typename T> int: position of object, -1 if not found

template <typename T> bool in_vector(vector<T> &vec, T obj)
{
  return std::find(vec.begin(), vec.end(), obj) != vec.end();
}

// char_string: converts a char to a string correctly
//
// char c: the char
//
// return string: a string of length 1

string char_string(char c)
{
  string s;
  s.push_back(c);
  return s;
}

// TokenStream::checkEOF: complains if 'source' is not consumed

void TokenStream::checkEOF()
{
  if (next() != "eof") {
    string err = "Parsing failed to consume tokens (" +
                 to_string(tokens.size()) + " remaining):\n";
    while (tokens.size() > 0) {
      err += advance() + "\n";
    }
    throw ParseError(err);
  }
}

// TokenStream::vomit: Spits out all unchomped characters
//
// return string: a string

string TokenStream::vomit()
{
  string out{"Remaining Tokens:\n"};
  for (string tk : tokens)
    out += "'" + tk + "'\n";
  return out;
}

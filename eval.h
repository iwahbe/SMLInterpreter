#pragma once
#include "parser.h"
#include "tokenstream.h"
#include <iostream>
#include <memory>

union Value {
  int i;
  bool b;
};

extern std::vector<std::string> INTOPS;
extern std::vector<std::string> CMPOPS;

class SMLValue {
    public:
  static std::shared_ptr<SMLValue> New(void);
  string type(void);
  void setTag(string t);
  virtual string to_string(void);
  virtual string to_string_typed(void);

    protected:
  SMLValue(void) { tag = "Uninitialized"; }
  string tag;
};

struct Binding {
  string name;
  std::shared_ptr<SMLValue> value;
};

class Environment {
    public:
  Environment() {}
  Environment add(string name, std::shared_ptr<SMLValue> v);
  std::shared_ptr<SMLValue> lookup(string x, string err);
  Environment(const Environment &t);
  std::string to_string(void);

    protected:
  vector<Binding> hold{}; // this would make a great hash map
  bool in_hold(string x);
};

class SMLClos : public SMLValue {
    public:
  static std::shared_ptr<SMLClos>
  New(std::string var, std::shared_ptr<AstNode> last, Environment env);
  static std::shared_ptr<SMLClos> New(std::shared_ptr<SMLValue> v);
  std::shared_ptr<SMLValue> eval(std::shared_ptr<SMLValue> x);
  std::string to_string(void);
  void envSet(Environment env_);

    protected:
  std::shared_ptr<AstNode> last;
  Environment env;
  std::string var;
  SMLClos(std::shared_ptr<AstNode> last_, Environment envr, std::string var_);
};

class SMLUnit : public SMLValue {
    public:
  static std::shared_ptr<SMLUnit> New(void);
  string to_string(void);

    protected:
  SMLUnit(void);
};

class SMLBool : public SMLValue {
    public:
  operator bool() const { return tfval; }
  string to_string(void) override;

  static std::shared_ptr<SMLBool> New(std::shared_ptr<class SMLValue> v);
  static std::shared_ptr<SMLBool> New(bool b);

    protected:
  SMLBool(bool b);
  bool tfval;
};

class SMLPair : public SMLValue {
    public:
  static std::shared_ptr<SMLPair> New(std::shared_ptr<SMLValue> right,
                                      std::shared_ptr<SMLValue> left);

  static std::shared_ptr<SMLPair> New(std::shared_ptr<class SMLValue> v,
                                      string msg);

  string to_string(void) override;
  string to_string_typed(void) override;

  std::shared_ptr<SMLValue> first();
  std::shared_ptr<SMLValue> last();

    protected:
  SMLPair(std::shared_ptr<SMLValue> r, std::shared_ptr<SMLValue> l);
  SMLPair(SMLValue *r, SMLValue *l);
  std::shared_ptr<SMLValue> rs;
  std::shared_ptr<SMLValue> ls;
};

class SMLInt : public SMLValue {
    public:
  static std::shared_ptr<SMLInt> New(int n)
  {
    return std::shared_ptr<SMLInt>(new SMLInt(n));
  }

  static std::shared_ptr<SMLInt> New(std::shared_ptr<class SMLValue> v,
                                     string msg)
  {
    if (v->type() != "Int") throw ParseError(msg);
    return std::dynamic_pointer_cast<SMLInt>(v);
  }

  std::shared_ptr<SMLInt> operator+(SMLInt const &right)
  {
    return SMLInt::New(this->val + right.val);
  }

  std::shared_ptr<SMLInt> operator-(SMLInt const &right)
  {
    return SMLInt::New(this->val - right.val);
  }

  std::shared_ptr<SMLInt> operator*(SMLInt const &right)
  {
    return SMLInt::New(this->val * right.val);
  }

  std::shared_ptr<SMLInt> operator/(SMLInt const &right)
  {
    return SMLInt::New(this->val / right.val);
  }

  std::shared_ptr<SMLInt> operator%(SMLInt const &right)
  {
    return SMLInt::New(this->val % right.val);
  }

  std::shared_ptr<SMLBool> operator==(SMLInt const &right)
  {
    return SMLBool::New(this->val == right.val);
  }

  std::shared_ptr<SMLBool> operator<(SMLInt const &right)
  {
    return SMLBool::New(this->val < right.val);
  }

  string to_string(void) override { return std::to_string(val); }

    protected:
  SMLInt(int n);
  int val;
};

std::shared_ptr<SMLValue> eval(Environment env, std::shared_ptr<AstNode> last);

std::shared_ptr<SMLValue> eval(std::shared_ptr<AstNode> last);

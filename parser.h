#pragma once
#include "tokenstream.h"
#include <iostream>

enum Label {
  If,
  Val,
  Fun,
  Funs,
  Lam,
  Let,
  Or,
  And,
  Less,
  Equals,
  Plus,
  Minus,
  Times,
  Div,
  Mod,
  App,
  Not,
  Print,
  First,
  Second,
  Literal,
  PairUp,
  Seq,
  Var,
  Unit,
  Int,
  Bool,
  String,
};

std::string label_to_string(Label l);

class AstNode {
    public:
  virtual bool is_branch() { return false; }
  virtual bool is_leaf() { return false; }
  virtual bool is_string() { return false; }
  std::string string_label();
  Label label() { return _label; }        // getter
  void label(Label lbl) { _label = lbl; } // setter
  virtual std::string to_string() { return "__UNDEFINED__"; };

    protected:
  Label _label;
};

class AstLeaf : public AstNode {
    public:
  bool is_leaf() override { return true; }
  int val() { return _val; }
  void val(int n) { _val = n; }
  static std::shared_ptr<AstLeaf> New(int n, Label l)
  {
    return std::shared_ptr<AstLeaf>(new AstLeaf(n, l));
  }

    protected:
  int _val;
  AstLeaf(int n, Label l)
  {
    _val = n;
    _label = l;
  }
};

class AstString : public AstNode {
    public:
  bool is_string() override { return true; }
  std::string get_string() { return _string; }
  void set_string(std::string s) { _string = s; }
  std::string to_string() override { return "\"" + _string + "\""; }
  static std::shared_ptr<AstString> New(std::string s)
  {
    return std::shared_ptr<AstString>(new AstString(s));
  }

    protected:
  AstString(std::string s)
  {
    _label = Label::String;
    _string = s;
  }
  std::string _string;
};

class AstBranch : public AstNode {
    public:
  AstBranch() {}
  std::string where(void);
  bool is_branch() override { return true; }
  void where(std::string whr);
  void add(std::shared_ptr<AstNode> node); // ads any AstNode subclass
  void add(std::string str);               // adds a string
  void add(Label label, int val);          // adds a leaf

  std::shared_ptr<AstNode> get(int n);
  int count(void);

  // for diagnosis
  std::string to_string() override;

    private:
  std::vector<std::shared_ptr<class AstNode>> args;
  std::string _where;
};

class SMLParser {
    public:
  SMLParser(TokenStream *tokens)
  {
    BINOPS = {"andalso", "orelse", "<", "=", "+", "-", "*", "div", "mod"};
    STOPPERS = {"then", "else", "in", "and", "end", ")", ";", ",", "eof"};
    STOPPERS.insert(STOPPERS.end(), BINOPS.begin(), BINOPS.end());
    MULTOPPS = {"*", "div", "mod"};
    ADDNOPPS = {"+", "-"};
    tks = tokens;
  }
  std::shared_ptr<AstBranch> operator()() { return parseExpn(); }

    private:
  std::shared_ptr<AstBranch> parseExpn(void);
  std::shared_ptr<AstBranch> parseDisj(void);
  std::shared_ptr<AstBranch> parseConj(void);
  std::shared_ptr<AstBranch> parseCmpn(void);
  std::shared_ptr<AstBranch> parseAddn(void);
  std::shared_ptr<AstBranch> parseMult(void);
  std::shared_ptr<AstBranch> parseAppl(void);
  std::shared_ptr<AstBranch> parsePrfx(void);
  std::shared_ptr<AstBranch> parseAtom(void);
  std::vector<std::string> BINOPS;
  std::vector<std::string> STOPPERS;
  std::vector<std::string> MULTOPPS;
  std::vector<std::string> ADDNOPPS;
  TokenStream *tks;
};

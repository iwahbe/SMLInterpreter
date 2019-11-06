#include "parser.h"

std::string AstNode::string_label(void) { return label_to_string(_label); }

void AstBranch::add(std::shared_ptr<AstNode> node) { args.push_back(node); }

void AstBranch::add(Label label, int value) { add(AstLeaf::New(value, label)); }

void AstBranch::add(std::string str) { args.push_back(AstString::New(str)); }

std::shared_ptr<AstNode> AstBranch::get(int n) { return args.at(n); }

int AstBranch::count(void) { return args.size(); }

std::string AstBranch::where(void) { return _where; }

void AstBranch::where(std::string whr) { _where = whr; }

// This is the actual parser part
std::shared_ptr<AstBranch> SMLParser::parseExpn(void)
{
  if (tks->next() == "")
    throw ParseError("SMLParser::parseExpn called on the empty string\n");
  std::string where = tks->report();
  //
  // <expn> ::= let val <name> = <expn> in <expn> end
  //          | if <expn> then <expn> else <expn>
  //          | fn <name> => <expn>
  std::shared_ptr<AstBranch> out = std::shared_ptr<AstBranch>(new AstBranch);
  if (tks->next() == "if") {
    tks->eat("if");
    std::shared_ptr<AstBranch> e0 = parseExpn();
    tks->eat("then");
    std::shared_ptr<AstBranch> e1 = parseExpn();
    tks->eat("else");
    std::shared_ptr<AstBranch> e2 = parseExpn();
    out->add(e0);
    out->add(e1);
    out->add(e2);
    out->label(Label::If);
    out->where(where);
    return out;
  }
  else if (tks->next() == "let") {
    tks->eat("let");
    std::shared_ptr<AstBranch> d =
        std::shared_ptr<AstBranch>(new AstBranch); // used in if and else
    if (tks->next() == "val") {
      tks->eat("val");
      std::string where_x = tks->report();
      std::string x = tks->eatName();
      tks->eat("=");
      std::shared_ptr<AstBranch> r = parseExpn();
      d->label(Label::Val);
      d->where(where_x);
      d->add(x);
      d->add(r);
    }
    else {
      tks->eat("fun");
      std::string where_f = tks->report();
      std::string f = tks->eatName();
      std::string x = tks->eatName();
      tks->eat("=");
      std::shared_ptr<AstBranch> r = parseExpn();
      d->label(Label::Fun);
      d->add(f);
      d->add(x);
      d->add(r);
      d->where(where_f);
      while (tks->next() == "and") {
        std::shared_ptr<AstBranch> dtmp =
            std::shared_ptr<AstBranch>(new AstBranch); // for use in the loop
        std::string where_and = tks->report();
        tks->eat("and");
        std::string where_f = tks->report();
        std::string f = tks->eatName();
        std::string x = tks->eatName();
        tks->eat("=");
        std::shared_ptr<AstBranch> r = parseExpn();
        // newing dp
        std::shared_ptr<AstBranch> dp =
            std::shared_ptr<AstBranch>(new AstBranch);
        dp->label(Label::Fun);
        dp->add(f);
        dp->add(x);
        dp->add(r);
        dp->where(where_f);
        // newing d fails as d is defined recursively on itself
        dtmp->label(Label::Funs);
        dtmp->add(d);
        dtmp->add(dp);
        dtmp->where(where_and);
        d = dtmp;
      }
    }
    tks->eat("in");
    std::shared_ptr<AstBranch> b = parseExpn();
    std::shared_ptr<AstBranch> out = std::shared_ptr<AstBranch>(new AstBranch);
    out->label(Label::Let);
    out->add(d);
    out->add(b);
    out->where(where);
    tks->eat("end");
    return out;
  }
  else if (tks->next() == "fn") {
    tks->eat("fn");
    std::string x = tks->eatName();
    tks->eat("=>");
    std::shared_ptr<AstBranch> r = parseExpn();
    std::shared_ptr<AstBranch> out = std::shared_ptr<AstBranch>(new AstBranch);
    out->label(Label::Lam);
    out->add(x);
    out->add(r);
    out->where(where);
    return out;
  }
  else {
    out = parseDisj();
    return out;
  }
}

std::shared_ptr<AstBranch> SMLParser::parseDisj(void)
{
  std::shared_ptr<AstBranch> e = parseConj();
  std::shared_ptr<AstBranch> tmp, ep;
  std::string where;
  while (tks->next() == "orelse") {
    tmp = std::shared_ptr<AstBranch>(new AstBranch);
    where = tks->report();
    tks->eat("orelse");
    ep = parseConj();
    tmp->label(Label::Or);
    tmp->add(e);
    tmp->add(ep);
    tmp->where(where);
    e = tmp;
  }
  return e;
}

std::shared_ptr<AstBranch> SMLParser::parseConj(void)
{
  std::shared_ptr<AstBranch> e = parseCmpn();
  std::string where;
  std::shared_ptr<AstBranch> tmp, ep;
  while (tks->next() == "andalso") {
    tmp = std::shared_ptr<AstBranch>(new AstBranch);
    where = tks->report();
    tks->eat("andalso");
    ep = parseCmpn();
    tmp->label(Label::And);
    tmp->add(e);
    tmp->add(ep);
    tmp->where(where);
    e = tmp;
  }
  return e;
}

std::shared_ptr<AstBranch> SMLParser::parseCmpn(void)
{
  std::shared_ptr<AstBranch> e = parseAddn();
  std::shared_ptr<AstBranch> ep;
  std::shared_ptr<AstBranch> tmp = std::shared_ptr<AstBranch>(new AstBranch);
  std::string where = tks->report();
  if (tks->next() == "<") {
    tks->eat("<");
    ep = parseAddn();
    tmp->label(Label::Less);
    tmp->add(e);
    tmp->add(ep);
    tmp->where(where);
    e = tmp;
  }
  else if (tks->next() == "=") {
    tks->eat("=");
    ep = parseAddn();
    tmp->label(Label::Equals);
    tmp->add(e);
    tmp->add(ep);
    tmp->where(where);
    e = tmp;
  }
  return e;
}

std::shared_ptr<AstBranch> SMLParser::parseAddn(void)
{
  std::shared_ptr<AstBranch> e = parseMult();
  std::shared_ptr<AstBranch> ep, tmp;
  std::string where;
  while (in_vector<std::string>(ADDNOPPS, tks->next())) {
    where = tks->report();
    tmp = std::shared_ptr<AstBranch>(new AstBranch);
    if (tks->next() == "+") {
      tks->eat("+");
      ep = parseMult();
      tmp->label(Label::Plus);
      tmp->add(e);
      tmp->add(ep);
      tmp->where(where);
      e = tmp;
    }
    else if (tks->next() == "-") {
      tks->eat("-");
      ep = parseMult();
      tmp->label(Label::Minus);
      tmp->add(e);
      tmp->add(ep);
      tmp->where(where);
      e = tmp;
    }
  }
  return e;
}

std::shared_ptr<AstBranch> SMLParser::parseMult(void)
{
  //
  // <mult> ::= <mult> * <nega> | <nega>
  //
  if (tks->next() == "")
    throw ParseError("SMLParser::parseMult called on the empty string\n");
  std::shared_ptr<AstBranch> e = parseAppl();
  std::shared_ptr<AstBranch> ep, tmp;
  std::string where;
  while (in_vector<std::string>(MULTOPPS, tks->next())) {
    where = tks->report();
    if (tks->next() == "*") {
      tks->eat("*");
      ep = parseAppl();
      tmp = std::shared_ptr<AstBranch>(new AstBranch);
      tmp->label(Label::Times);
      tmp->add(e);
      tmp->add(ep);
      tmp->where(where);
      e = tmp;
    }
    else if (tks->next() == "div") {
      tks->eat("div");
      ep = parseAppl();
      tmp = std::shared_ptr<AstBranch>(new AstBranch);
      tmp->label(Label::Div);
      tmp->add(e);
      tmp->add(ep);
      tmp->where(where);
      e = tmp;
    }
    else if (tks->next() == "mod") {
      tks->eat("mod");
      ep = parseAppl();
      tmp = std::shared_ptr<AstBranch>(new AstBranch);
      tmp->label(Label::Mod);
      tmp->add(e);
      tmp->add(ep);
      tmp->where(where);
      e = tmp;
    }
    else {
      throw ParseError("SMLParser reached a bad place!");
    }
  }
  return e;
}

std::shared_ptr<AstBranch> SMLParser::parseAppl(void)
{
  //
  // <appl> ::= <appl> <nega> | <nega>
  //
  if (tks->next() == "")
    throw ParseError("SMLParser::parseAppl called on the empty string\n");
  std::shared_ptr<AstBranch> e = parsePrfx();
  while (!in_vector<std::string>(STOPPERS, tks->next())) {
    std::string where = tks->report();
    std::shared_ptr<AstBranch> ep = parsePrfx();
    std::shared_ptr<AstBranch> tmp = std::shared_ptr<AstBranch>(new AstBranch);
    tmp->label(Label::App);
    tmp->add(e);
    tmp->add(ep);
    tmp->where(where);
    e = tmp;
  }
  return e;
}

std::shared_ptr<AstBranch> SMLParser::parsePrfx(void)
{
  if (tks->next() == "")
    throw ParseError("SMLParser::parsePrfx called on the empty string\n");
  //
  // <atom> ::= not <atom> | print <atom> | <atom>
  //          | fst <atom> | snd <atom>
  //
  std::shared_ptr<AstBranch> e, out;
  std::string where = tks->report();
  std::string tst_ = tks->next();
  if (tst_ == "not") {
    tks->eat("not");
    e = parseAtom();
    out = std::shared_ptr<AstBranch>(new AstBranch);
    out->label(Label::Not);
    out->add(e);
    out->where(where);
    return out;
  }

  else if (tst_ == "print") {
    tks->eat("print");
    e = parseAtom();
    out = std::shared_ptr<AstBranch>(new AstBranch);
    out->label(Label::Print);
    out->add(e);
    out->where(where);
    return out;
  }

  else if (tst_ == "fst") {
    tks->eat("fst");
    e = parseAtom();
    out = std::shared_ptr<AstBranch>(new AstBranch);
    out->label(Label::First);
    out->add(e);
    out->where(where);
    return out;
  }

  else if (tst_ == "snd") {
    tks->eat("snd");
    e = parseAtom();
    out = std::shared_ptr<AstBranch>(new AstBranch);
    out->label(Label::Second);
    out->add(e);
    out->where(where);
    return out;
  }

  else {
    return parseAtom();
  }
}

std::shared_ptr<AstBranch> SMLParser::parseAtom(void)
{
  //
  // <atom> ::= 375
  //
  std::string where;
  std::shared_ptr<AstBranch> out = std::shared_ptr<AstBranch>(new AstBranch);

  if (tks->next() == "")
    throw SyntaxError("SMLParser::parseAtom was fed the empty string at " +
                      tks->report() + "\n");

  if (tks->nextIsInt()) {
    where = tks->report();
    int n = tks->eatInt();
    out->label(Label::Literal);
    out->add(Label::Int, n);
    out->where(where);
    return out;
  }
  //
  // <atom> ::= () | ( <expn> )
  //          | ( <expn> ; ... ; <expn> )
  //          | ( <expn> , <expn> )
  //
  else if (tks->next() == "(") {
    std::shared_ptr<AstBranch> e, ep;
    tks->eat("(");
    // unit literal
    if (tks->next() == ")") {
      e->label(Label::Literal);
      e->add(Label::Unit, 0);
      e->where(tks->report());
    }

    else {
      e = parseExpn();
      // pairing up
      if (tks->next() == ",") {
        where = tks->report();
        tks->eat(",");
        ep = parseExpn();
        out->label(Label::PairUp);
        out->add(e);
        out->add(ep);
        out->where(where);
        e = out;
      }

      else
        // Sequencing
        while (tks->next() == ";") {
          out = std::shared_ptr<AstBranch>(new AstBranch);
          where = tks->report();
          tks->eat(";");
          ep = parseExpn();
          // Out is not out this run
          out->label(Label::Seq);
          out->add(e);
          out->add(ep);
          out->where(where);
          e = out;
        }
    }
    tks->eat(")");
    return e;
  }

  //
  // <atom> ::= <name>
  //
  else if (tks->nextIsName()) {
    std::string where = tks->report();
    out->label(Label::Var);
    std::string name = tks->eatName();
    out->add(name);
    out->where(where);
    return out;
  }
  //
  // <atom> ::= true
  //
  else if (tks->next() == "true") {
    tks->eat("true");
    out->label(Label::Literal);
    out->add(Label::Bool, 1);
    out->where(tks->report());
    return out;
  }
  //
  // <atom> ::= true
  //
  else if (tks->next() == "false") {
    tks->eat("false");
    out->label(Label::Literal);
    out->add(Label::Bool, 0);
    out->where(tks->report());
    return out;
  }
  // not known combination: complain
  else {
    std::string err;
    err = "Unexpected token at " + tks->report() + ". " + "Saw: '" +
          tks->next() + "'. ";
    throw SyntaxError(err);
  }
}

std::string AstBranch::to_string(void)
{
  std::string out;
  out = "[" + string_label() + ", ";
  for (int i = 0; i < args.size(); i++) {
    out += args.at(i)->to_string();
  }
  out += ", " + where() + "] ";
  return out;
}

std::string label_to_string(Label l)
{
  switch (l) {
  case Label::If:
    return "If";
  case Label::Val:
    return "Val";
  case Label::Fun:
    return "Fun";
  case Label::Funs:
    return "Funs";
  case Label::Lam:
    return "Lam";
  case Label::Let:
    return "Let";
  case Label::Or:
    return "Or";
  case Label::And:
    return "And";
  case Label::Less:
    return "Less";
  case Label::Equals:
    return "Equals";
  case Label::Plus:
    return "Plus";
  case Label::Minus:
    return "Minus";
  case Label::Times:
    return "Times";
  case Label::Div:
    return "Div";
  case Label::Mod:
    return "Mod";
  case Label::App:
    return "App";
  case Label::Not:
    return "Not";
  case Label::Print:
    return "Print";
  case Label::First:
    return "First";
  case Label::Second:
    return "Second";
  case Label::Literal:
    return "Literal";
  case Label::PairUp:
    return "PairUp";
  case Label::Seq:
    return "Seq";
  case Label::Var:
    return "Var";
  case Label::Int:
    return "Int";
  case Label::Bool:
    return "Bool";
  case Label::Unit:
    return "Unit";
  case Label::String:
    return "String";
  }
}

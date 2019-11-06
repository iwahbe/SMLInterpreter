#include "eval.h"

std::vector<std::string> INTOPS = {"Plus", "Minus", "Times", "Div", "Mod"};
std::vector<std::string> CMPOPS = {"Equals", "Less"};

std::shared_ptr<SMLValue> SMLValue::New(void)
{
  return std::shared_ptr<SMLValue>(new SMLValue());
}

string SMLValue::type(void) { return tag; }
void SMLValue::setTag(string t) { tag = t; }
string SMLValue::to_string(void) { return "(SMLValue)"; }
string SMLValue::to_string_typed(void)
{
  return "[" + tag + ", " + to_string() + "]";
}

Environment Environment::add(string name, std::shared_ptr<SMLValue> v)
{
  Binding tmp{name, v};
  Environment out = Environment(*this);
  out.hold.push_back(tmp);
  return out;
}

std::shared_ptr<SMLValue> Environment::lookup(string x, string err)
{
  for (int i = 0; i < hold.size(); i++)
    if (hold.at(i).name == x) return hold.at(i).value;
  string current_vars{""};
  for (int i = 0; i < hold.size(); i++)
    current_vars += hold.at(i).name + "\n";
  throw RunTimeError("Use of variable '" + x + "'. " + "\n" +
                     this->to_string());
}

Environment::Environment(const Environment &t) { hold = t.hold; }

bool Environment::in_hold(string x)
{
  for (int i = 0; i < hold.size(); i++)
    if (hold.at(i).name == x) return true;
  return false;
}

string Environment::to_string()
{
  std::string out{"Environment{ \n"};

  for (int i = 0; i < hold.size(); i++)
    out +=
        "\t(" + hold.at(i).name + "," + hold.at(i).value->to_string() + ")\n";
  out += "}\n";
  return out;
}

std::shared_ptr<SMLClos> SMLClos::New(std::string var,
                                      std::shared_ptr<AstNode> ast_node,
                                      Environment env)
{
  return std::shared_ptr<SMLClos>(new SMLClos(ast_node, env, var));
}
std::shared_ptr<SMLClos> SMLClos::New(std::shared_ptr<SMLValue> v)
{
  if (v->type() != "Clos")
    throw ParseError("Bad Clos cast: tried to cast type " + v->type());
  else
    return std::dynamic_pointer_cast<SMLClos>(v);
}
std::shared_ptr<SMLValue> SMLClos::eval(std::shared_ptr<SMLValue> x)
{
  return ::eval(env.add(var, x), last);
}

SMLClos::SMLClos(std::shared_ptr<AstNode> last_, Environment envr,
                 std::string var_)
{
  last = last_;
  env = envr;
  var = var_;
  tag = "Clos";
}

void SMLClos::envSet(Environment env_) { env = env_; }

std::string SMLClos::to_string(void)
{
  return "[" + var + " => " + last->to_string() + "]";
}

std::shared_ptr<SMLUnit> SMLUnit::New(void)
{
  return std::shared_ptr<SMLUnit>(new SMLUnit());
}

string SMLUnit::to_string(void) { return "()"; }

SMLUnit::SMLUnit(void) { tag = "Unit"; }

string SMLBool::to_string(void) { return (*this ? "true" : "false"); }

std::shared_ptr<SMLBool> SMLBool::New(std::shared_ptr<class SMLValue> v)
{
  if (v->type() == "Bool")
    return std::dynamic_pointer_cast<SMLBool>(v);
  else
    throw ParseError("Bad Bool cast " + v->to_string());
}

std::shared_ptr<SMLBool> SMLBool::New(bool b)
{
  return std::shared_ptr<SMLBool>(new SMLBool(b));
}

SMLBool::SMLBool(bool b)
{
  tag = "Bool";
  tfval = b;
}

std::shared_ptr<SMLPair> SMLPair::New(std::shared_ptr<SMLValue> right,
                                      std::shared_ptr<SMLValue> left)
{
  return std::shared_ptr<SMLPair>(new SMLPair(right, left));
}

std::shared_ptr<SMLPair> SMLPair::New(std::shared_ptr<class SMLValue> v,
                                      string msg)
{
  if (v->type() != "PairUp") throw RunTimeError("Bad Pair Cast: " + msg);
  return std::dynamic_pointer_cast<SMLPair>(v);
} // for already the right type

string SMLPair::to_string(void)
{
  return "(" + rs->to_string() + "," + ls->to_string() + ")";
}
string SMLPair::to_string_typed(void)
{
  return "[Pair, (" + rs->to_string_typed() + ", " + ls->to_string_typed() +
         ")]";
}

std::shared_ptr<SMLValue> SMLPair::first()
{
  return rs;
} // needs to return a shared
std::shared_ptr<SMLValue> SMLPair::last() { return ls; }

SMLPair::SMLPair(std::shared_ptr<SMLValue> r, std::shared_ptr<SMLValue> l)
{
  rs = r;
  ls = l;
  tag = "PairUp";
}
SMLPair::SMLPair(SMLValue *r, SMLValue *l)
{
  std::shared_ptr<SMLValue> rs = std::shared_ptr<SMLValue>(r);
  std::shared_ptr<SMLValue> ls = std::shared_ptr<SMLValue>(l);
  tag = "PairUp";
}

SMLInt::SMLInt(int n)

{
  val = n;
  tag = "Int";
}

// eval: The work horse of the eval engine
//
// Environment env: The Environment within which to find vars
// Astnode last: The AST holder to evaluate, could also hold a literal
//
// return std::shared_ptr<SMLValue>: The ending value

std::shared_ptr<SMLValue> eval(Environment env, std::shared_ptr<AstNode> last)
{
  if (last->is_leaf()) {
    std::shared_ptr<AstLeaf> leaf = std::dynamic_pointer_cast<AstLeaf>(last);
    if (leaf->label() == Label::Bool) {
      return SMLBool::New(leaf->val());
    }
    else if (leaf->label() == Label::Int) {
      return SMLInt::New(leaf->val());
    }
    else
      throw ParseError("found new literal type!");
  }
  else if (last->is_branch()) {
    std::shared_ptr<AstBranch> ast = std::dynamic_pointer_cast<AstBranch>(last);

    if (ast->label() == Label::If) {
      std::shared_ptr<SMLBool> v0 = SMLBool::New(eval(env, ast->get(0)));
      string err = "Type error in condition at " + ast->where() +
                   ". Expected a boolean value.";
      if (*v0)
        return eval(env, ast->get(1));
      else
        return eval(env, ast->get(2));
    }
    else if (ast->label() == Label::Literal) {
      return eval(env, ast->get(0));
    }
    else if (ast->label() == Label::Let) {

      std::shared_ptr<AstNode> b;
      std::shared_ptr<AstBranch> d =
          std::dynamic_pointer_cast<AstBranch>(ast->get(0));
      b = ast->get(1);
      Environment envp = env;
      if (!(d->is_branch()))
        throw ParseError("expected ast");

      else if (d->label() == Label::Val) {
        string x =
            std::dynamic_pointer_cast<AstString>(d->get(0))->get_string();
        std::shared_ptr<AstNode> r = d->get(1);
        std::shared_ptr<SMLValue> u = eval(env, r);
        envp = env.add(x, u);
      }

      else if (d->label() == Label::Fun) {
        std::shared_ptr<AstBranch> fun = d;
        std::shared_ptr<SMLClos> c = SMLClos::New(
            std::dynamic_pointer_cast<AstString>(fun->get(1))->get_string(),
            fun->get(2), env);
        envp = env.add(
            std::dynamic_pointer_cast<AstString>(fun->get(0))->get_string(), c);
        c->envSet(envp);
      }

      else if (d->label() == Label::Funs) {
        std::shared_ptr<AstBranch> funs = d;
        std::shared_ptr<AstBranch> fun;
        std::shared_ptr<SMLClos> c;
        std::vector<std::shared_ptr<SMLClos>> to_set;
        do { // eat second half
          fun = std::dynamic_pointer_cast<AstBranch>(funs->get(1));
          c = SMLClos::New(
              std::dynamic_pointer_cast<AstString>(fun->get(1))->get_string(),
              fun->get(2), envp);
          envp = envp.add(
              std::dynamic_pointer_cast<AstString>(fun->get(0))->get_string(),
              c);
          to_set.push_back(c);

        } while (funs->get(0)->label() == Label::Funs &&
                 (funs = std::dynamic_pointer_cast<AstBranch>(funs->get(0))));
        fun = std::dynamic_pointer_cast<AstBranch>(funs->get(0));
        c = SMLClos::New(
            std::dynamic_pointer_cast<AstString>(fun->get(1))->get_string(),
            fun->get(2), envp);
        envp = envp.add(
            std::dynamic_pointer_cast<AstString>(fun->get(0))->get_string(), c);
        to_set.push_back(c);
        for (std::shared_ptr<SMLClos> c : to_set)
          c->envSet(envp);
      }

      else
        throw ParseError("Tried to call Let on " + d->to_string());
      return eval(envp, b);
    }
    else if (ast->label() == Label::Lam) {
      return SMLClos::New(
          std::dynamic_pointer_cast<AstString>(ast->get(0))->get_string(),
          ast->get(1), env);
    }
    else if (ast->label() == Label::App) {
      std::shared_ptr<SMLClos> v1 = SMLClos::New(eval(env, ast->get(0)));
      std::shared_ptr<SMLValue> v2 = eval(env, ast->get(1));
      return v1->eval(v2);
    }
    else if (ast->label() == Label::Var) {
      string x =
          std::dynamic_pointer_cast<AstString>(ast->get(0))->get_string();
      string err = "Unbound variable at " + ast->where() + ". ";
      return env.lookup(x, err);
    }
    else if (ast->label() == Label::Or) {
      std::shared_ptr<SMLBool> e0, e1;
      e0 = SMLBool::New(eval(env, ast->get(0)));
      if (*e0) {
        return e0;
      }
      else {
        e1 = SMLBool::New(eval(env, ast->get(1)));
        return e1;
      }
    }
    else if (ast->label() == Label::And) {
      std::shared_ptr<SMLBool> e0, e1;
      e0 = SMLBool::New(eval(env, ast->get(0)));
      if (!(*e0))
        return e0;
      else {
        e1 = SMLBool::New(eval(env, ast->get(1)));
        return e1;
      }
    }
    else if (in_vector(INTOPS, ast->string_label())) {
      std::shared_ptr<SMLInt> v1, v2;
      std::shared_ptr<SMLValue> val1, val2;
      // better return SMLInt
      val1 = eval(env, ast->get(0));
      v1 = SMLInt::New(val1, "INTOPS v1: " + val1->to_string_typed());
      val2 = eval(env, ast->get(1));
      v2 = SMLInt::New(val2, "INTOPS v2: " + val2->to_string_typed());
      if (ast->label() == Label::Plus) {
        return *v1 + *v2;
      }
      else if (ast->label() == Label::Minus)
        return *v1 - *v2;
      else if (ast->label() == Label::Times)
        return *v1 * *v2;
      else if (ast->label() == Label::Div)
        return *v1 / *v2;
      else if (ast->label() == Label::Mod)
        return *v1 % *v2;
      else
        throw ParseError("Attempted operation on invalid operator" +
                         ast->string_label());
    }
    else if (in_vector(CMPOPS, ast->string_label())) { // FIXME

      std::shared_ptr<SMLInt> v1, v2;
      // better return SMLInt
      v1 = SMLInt::New(eval(env, ast->get(0)), "CMPOPS v1: ");
      v2 = SMLInt::New(eval(env, ast->get(1)), "CMPOPS v2: ");
      if (ast->label() == Label::Less)
        return *v1 < *v2;
      else if (ast->label() == Label::Equals)
        return *v1 == *v2;
      else
        throw ParseError("Attempted operation on invalid operator" +
                         ast->string_label());
    }
    else if (ast->label() == Label::PairUp) {
      return SMLPair::New(eval(env, ast->get(0)), eval(env, ast->get(1)));
    }
    else if (ast->label() == Label::First) {
      return SMLPair::New(
                 eval(env, ast->get(0)),
                 "Attempted to extract 1st component of a non-pair at " +
                     ast->where() + ".")
          ->first();
    }
    else if (ast->label() == Label::Second) {
      return SMLPair::New(
                 eval(env, ast->get(0)),
                 "Attempted to extract 2nd component of a non-pair at " +
                     ast->where() + ".")
          ->last();
    }
    else if (ast->label() == Label::Seq) {
      eval(env, ast->get(0));
      return eval(env, ast->get(1));
    }
    else if (ast->label() == Label::Print) {
      std::shared_ptr<SMLValue> tv;
      tv = eval(env, ast->get(0));
      std::cout << tv->to_string();
      return SMLUnit::New();
    }
    else if (ast->label() == Label::Not) {
      return SMLBool::New(!(*SMLBool::New(eval(env, ast->get(0)))));
    }
    else {
      throw NotImplemented("Found unimplemented type of AST: \"" +
                           ast->string_label() + "\"");
    }
  }
  else if (last->is_string()) {
    throw ParseError("String leaf not handled!");
  }
  else {
    throw ParseError("Badly constructed raw AstNode");
  }
}

std::shared_ptr<SMLValue> eval(std::shared_ptr<AstNode> ast_node)
{
  return eval(Environment(), ast_node);
}

#include "eval.h"
#include <iostream>

void runTest(std::string const &entry, std::string const &result, int &testNum,
             int &tests_not_implemented, int &tests_passed)
{
  testNum++;
  std::cout << "\nTEST NO" << testNum << ": " << entry;
  try {
    InputStream i{entry};
    TokenStream t = TokenStream("", &i);
    std::shared_ptr<AstBranch> ast = SMLParser(&t)();
    std::shared_ptr<SMLValue> out = eval(ast);

    if (out->to_string() == result) {
      std::cout << " => " << result << ": passed.";
      tests_passed++;
    }
    else {
      std::cout << " => failed:\n\tExpected: " << result
                << "\n\tGot: " << out->to_string() << "\n";
    }
  }
  catch (NotImplemented err) {
    std::cout << " => " << err.what() << " on test " << testNum;
    tests_not_implemented++;
  }
  catch (LexError err) {
    std::cout << " => " << err.what() << " on test" << testNum;
  }
}

int unitTestAll(void)
{
  std::cout << "Running unit test on SML interpreter";
  int test_no{0};
  int test_not_implemented{0};
  int tests_passed{0};
  auto test = [&tests_passed, &test_not_implemented, &test_no](string i,
                                                               string o) {
    runTest(i, o, test_no, test_not_implemented, tests_passed);
  };

  test("5", "5");                                                   // 1
  test("5+1", "6");                                                 // 2
  test("5-1", "4");                                                 // 3
  test("3*4", "12");                                                // 4
  test("37 mod 10", "7");                                           // 5
  test("37 div 10", "3");                                           // 6
  test("print 5", "()");                                            // 7
  test("(5;6;7;8)", "8");                                           // 8
  test("(print 5;6)", "6");                                         // 9
  test("(2,3)", "(2,3)");                                           // 10
  test("fst (2,3)", "2");                                           // 11
  test("snd (2,3)", "3");                                           // 12
  test("true andalso false", "false");                              // 13
  test("false orelse true", "true");                                // 14
  test("false andalso (print 5; true)", "false");                   // 15
  test("true orelse (print 5; true)", "true");                      // 16
  test("let val x=5 in x+1 end", "6");                              // 17
  test("let val x=3 in let val y=x*2 in 10*x+y end end", "36");     // 18
  test("(fn x => x+1) 5", "6");                                     // 19
  test("(fn x => fn y => x*10+y) 3 4", "34");                       // 20
  test("let val f = fn x=> (x*2,x) in fst (f 3) end", "6");         // 21
  test("let val f = fn x=> (x*2,x) in snd (f 3) end", "3");         // 22
  test("(print (5+6); 8*9)", "72");                                 // 23
  test("let fun f x = if x=0 then 1 else x*(f (x-1)) in (f 5) end", // 24
       "120");
  test("let fun f x = fn y => if x=0 then y else f (x div 10) (y*10 + x mod "
       "10) in (f 315 0) end",
       "513"); // 25
  test("let fun f x = if x=0 then true else (g (x-1)) and g y = if y=0 "
       "then false else (f (y-1)) in (f 10, f 11) end",
       "(true,false)");                                     // 26
  test("(not (not true),not (not false))", "(true,false)"); // 27
  test("let fun fib x = if x=0 then 0 else if x=1 then 1 else (fib (x-1))+(fib "
       "(x-2)) in (fib 10) end",
       "55"); // 28
  std::cout << "\n"
            << tests_passed << " passed! "
            << test_no - tests_passed - test_not_implemented << " failed! "
            << test_not_implemented << " unimplemented!\n";
  return test_no - tests_passed; // number of tests failed
}

std::shared_ptr<SMLValue> interpret(TokenStream tks)
{
  std::shared_ptr<AstBranch> ast = SMLParser(&tks)();
  tks.checkEOF();
  std::shared_ptr<SMLValue> result = eval(ast);
  std::cout << "Out: " << result->to_string_typed() << "\n";
  return result;
}

int main(int argc, char **argv)
{
  if (argc == 2 && !strcmp(argv[1], "test")) {
    return unitTestAll();
  }
  else if (argc >= 2) {
    InputStream i;
    i.get_file(argv[1]);
    try {
      interpret(TokenStream(argv[1], &i));
    }
    catch (LexError err) {
      std::cout << "\nError caught:\n";
      std::cout << err.what() << "\n";
    }
  }
  else { // process typed line

    InputStream i;
    std::cout << "Type something, I dare you!\n"
                 "SML Input:\n";
    while (i.get_full_line())
      try {
        interpret(TokenStream("STDIN", &i));
      }
      catch (LexError err) {
        std::cout << "\nError caught:\n";
        std::cout << err.what() << "\n";
      }
    std::cout << "Functional programming will always be here!\n"
                 "No point running!\n";
  }
}

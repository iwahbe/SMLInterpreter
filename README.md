# SMLInterpreter
Partial interpreter for SML, written in C++.

This project began as part of an assignment from [Jim Fix](http://people.reed.edu/~jimfix/), relating to his class on languages. I ported a version of his parser, written in Python. 

The entry file for the program is miniml.cc. Currently, this project only acts as a dumb interpreter, and does not support loading files. 

Examples:
Calculate the 10th Fibonacci number:
```
let fun fib x = if x=0 then 0 else if x=1 then 1 else (fib (x-1))+(fib(x-2)) in (fib 10) end
```

Computes Collatz sequences for 15:
```
let fun clos x = if x=1 then 1 else if (x mod 2)=0 then clos (print (x div 2); clos (x div 2)) else (print (3*x+1); clos (3*x+1)) in (clos 15) end
```


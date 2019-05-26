#!/bin/bash
try() {
  expected="$1"
  input="$2"

  ./9cc "$input" > tmp.s
  gcc -o tmp tmp.s tmp-*.o
  ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$expected expected, but got $actual"
    exit 1
  fi
}
echo 'int foo() { printf("OK from foo\n"); }' | gcc -xc -c -o tmp-foo.o -
echo 'int two() { return 2; }' | gcc -xc -c -o tmp-two.o -
echo 'int plus(int x, int y) { return x + y; }' | gcc -xc -c -o tmp-plus.o -

try 0 'main() { return 0; }'
try 42 'main() { return 42; }'

try 21 'main() { return 5+20-4; }'
try 116 'main() { return 100+20-4; }'

try 120 'main() { return 100 + 20; }'
try 200 'main() { return 10 * 20; }'
try 20 'main() { return 200 / 10; }'

try 203 'main() { return 10 * 20 + 3; }'
try 70 'main() { return 10 + 20 * 3; }'

try 90 'main() { return (10 + 20) * 3; }'

try 90 'main() { return (+10 + 20) * 3; }'
try 30 'main() { return +(-10 + 20) * 3; }'
try 30 'main() { return -(-10 + 20) * - 3; }'

try 1 'main() { return 100 == 100; }'
try 0 'main() { return 10 == 1; }'

try 0 'main() { return 100 != 100; }'
try 1 'main() { return 10 != 12; }'

try 1 'main() { return 100 <= 1001; }'
try 0 'main() { return 10002 <= 1001; }'

try 1 'main() { return 100 < 1001; }'
try 0 'main() { return 10002 < 1001; }'

try 0 'main() { return 100 >= (10 * 100); }'
try 1 'main() { return 100 * (10 + 1) >= 1001; }'

try 0 'main() { return 100 > 1001; }'
try 1 'main() { return 10002 > 1001; }'

try 1 'main() { 10; return 10002 > 1001; }'

try 100 'main() { 3; int a; return a = 100; }'
try 103 'main() { int a; a = 3; return a + 100; }'
try 35 'main() { int a;int b; a = 3;b=5; return b + 10 * a ; }'

try 5 'main() { int a;int b; a = 3;b=5; return b;  b + 10 * a ; }'
try 3 'main() { int a; int b; return 3; a = 3;b=5; return b;  b + 10 * a ; }'

try 199 'main() { int one; int two; one=1; two = 2; return 100 * two - one; }'

try 100 'main() {if(1) return 100; 2; }'
try 101 'main() {if(1 == 3) return 100; if(100) return 101; return 1; }'

try 2 'main() { if(1) return 2; else return 3; return 4; }'
try 3 'main() { if(0) return 2; else return 3; return 4; }'

try 55 'main() {i=0; j=0; while(i<10) j = j + (i = i + 1); return j; }'

try 45 'main() {j=0; for(i=0; i<10; i = i+1) j = j + i; return j; }'
try 45 'main() {j=0; i=0; for(; i<10; i = i+1) j = j + i; return j; }'

try 55 'main() { i=0; j=0; while(i<10) { i=i+1;  j = j + i; } return j; }'
try 200 'main() { i = 0; if( i == 0 ){ i = 100; i = i * 2;  } return i; }'

try 100 'main() { foo(); return 100; }'
try 100 'main() { for(i=0; i<5; i=i+1) {foo();} return 100; }'
try 9 'main() { return 1 + two() * 4; }'

try 3 'main() { return plus(1,2); }'
try 4 'main() { return 1 + plus(1,2); }'
try 50 'main() { return 2 + plus(2,10) * 4; }'

try 5 'two() { return 2; } main() { return 2 *two() + 1; }'
try 5 'add3(x) { return x+3; } main() { return add3(2); }'
try 5 'add(x, y) { return x+y; } main() { return add(2, 3); }'
try 13 'fib(i) { if(i<=1) return 1; return fib(i-1) + fib(i-2); } main() { return fib(6); }'

echo OK

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

try 0 '0;'
try 42 '42;'

try 21 '5+20-4;'
try 116 '100+20-4;'

try 120 '100 + 20;'
try 200 '10 * 20;'
try 20 '200 / 10;'

try 203 '10 * 20 + 3;'
try 70 '10 + 20 * 3;'

try 90 '(10 + 20) * 3;'

try 90 '(+10 + 20) * 3;'
try 30 '+(-10 + 20) * 3;'
try 30 '-(-10 + 20) * -3;'

try 1 '100 == 100;'
try 0 '10 == 1;'

try 0 '100 != 100;'
try 1 '10 != 12;'

try 1 ' 100 <= 1001;'
try 0 ' 10002 <= 1001;'

try 1 ' 100 < 1001;'
try 0 ' 10002 < 1001;'

try 0 ' 100 >= (10 * 100);'
try 1 ' 100 * (10 + 1) >= 1001;'

try 0 ' 100 > 1001;'
try 1 ' 10002 > 1001;'

try 1 ' 10; 10002 > 1001;'

try 100 '3; a = 100;'
try 103 ' a = 3; a + 100;'
try 35 ' a = 3;b=5;  b + 10 * a ;'

try 5 ' a = 3;b=5; return b;  b + 10 * a ;'
try 3 ' return 3; a = 3;b=5; return b;  b + 10 * a ;'

try 199 'one=1; two = 2; return 100 * two - one;'

try 100 'if(1) return 100; 2;'
try 101 'if(1 == 3) return 100; if(100) return 101; return 1;'

try 2 'if(1) return 2; else return 3; return 4;'
try 3 'if(0) return 2; else return 3; return 4;'

try 55 'i=0; j=0; while(i<10) j = j + (i = i + 1); return j; '

try 45 'j=0; for(i=0; i<10; i = i+1) j = j + i; return j;'
try 45 'j=0; i=0; for(; i<10; i = i+1) j = j + i; return j;'

try 55 'i=0; j=0; while(i<10) { i=i+1;  j = j + i; } return j; '
try 200 'i = 0; if( i == 0 ){ i = 100; i = i * 2;  } return i;'

try 100 'foo(); return 100;'
try 100 'for(i=0; i<5; i=i+1) {foo();} return 100;'
try 9 '1 + two() * 4;'

try 3 'plus(1,2);'
try 4 '1 + plus(1,2);'
try 50 '2 + plus(2,10) * 4;'
echo OK

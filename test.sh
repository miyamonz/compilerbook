#!/bin/bash
try() {
  expected="$1"
  input="$2"

  ./9cc "$input" > tmp.s
  gcc -o tmp tmp.s
  ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$expected expected, but got $actual"
    exit 1
  fi
}

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
echo OK

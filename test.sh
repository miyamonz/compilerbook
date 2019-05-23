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

try 0 0
try 42 42

try 21 '5+20-4'
try 116 '100+20-4'

try 120 '100 + 20'
try 200 '10 * 20'
try 20 '200 / 10'

try 203 '10 * 20 + 3'
try 70 '10 + 20 * 3'

try 90 '(10 + 20) * 3'

try 90 '(+10 + 20) * 3'
try 30 '+(-10 + 20) * 3'
try 30 '-(-10 + 20) * -3'

try 1 '100 == 100'
try 0 '10 == 1'
echo OK

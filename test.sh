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
    echo "-----------------------------------"
    echo "$input"
    echo "$expected expected, but got $actual"
    exit 1
  fi
}
cat << 'EOF' | gcc -xc -c -o tmp-test.o -
int foo() {
  printf("OK from foo\n");
}
int two() { return 2; }
int plus(int x,int y) { return x+y; }

void debug(int x) {
  printf("debug: %d\n", x);
}

int *alloc(int x) {
  static int arr[1];
  arr[0] = x;
  return arr;
}
int *alloc1(int x, int y) {
  static int arr[2];
  arr[0] = x;
  arr[1] = y;
  return arr;
}
int *alloc2(int x, int y) {
  static int arr[2];
  arr[0] = x;
  arr[1] = y;
  return arr + 1;
}
int **alloc_ptr_ptr(int x) {
  static int **p;
  static int *q;
  static int r;
  r = x;
  q = &r;
  p = &q;
  return p;
}
EOF

try 0 'int main() { return 0; }'
try 42 'int main() { return 42; }'

try 21 'int main() { return 5+20-4; }'
try 116 'int main() { return 100+20-4; }'

try 120 'int main() { return 100 + 20; }'
try 200 'int main() { return 10 * 20; }'
try 20 'int main() { return 200 / 10; }'

try 203 'int main() { return 10 * 20 + 3; }'
try 70 'int main() { return 10 + 20 * 3; }'

try 90 'int main() { return (10 + 20) * 3; }'

try 90 'int main() { return (+10 + 20) * 3; }'
try 30 'int main() { return +(-10 + 20) * 3; }'
try 30 'int main() { return -(-10 + 20) * - 3; }'

try 1 'int main() { return 100 == 100; }'
try 0 'int main() { return 10 == 1; }'

try 0 'int main() { return 100 != 100; }'
try 1 'int main() { return 10 != 12; }'

try 1 'int main() { return 100 <= 1001; }'
try 0 'int main() { return 10002 <= 1001; }'

try 1 'int main() { return 100 < 1001; }'
try 0 'int main() { return 10002 < 1001; }'

try 0 'int main() { return 100 >= (10 * 100); }'
try 1 'int main() { return 100 * (10 + 1) >= 1001; }'

try 0 'int main() { return 100 > 1001; }'
try 1 'int main() { return 10002 > 1001; }'

try 1 'int main() { 10; return 10002 > 1001; }'

try 100 'int main() { 3; int a; return a = 100; }'
try 103 'int main() { int a; a = 3; return a + 100; }'
try 35 'int main() { int a;int b; a = 3;b=5; return b + 10 * a ; }'

try 5 'int main() { int a;int b; a = 3;b=5; return b;  b + 10 * a ; }'
try 3 'int main() { int a; int b; return 3; a = 3;b=5; return b;  b + 10 * a ; }'

try 199 'int main() { int one; int two; one=1; two = 2; return 100 * two - one; }'

try 100 'int main() {if(1) return 100; 2; }'
try 101 'int main() {if(1 == 3) return 100; if(100) return 101; return 1; }'

try 2 'int main() { if(1) return 2; else return 3; return 4; }'
try 3 'int main() { if(0) return 2; else return 3; return 4; }'

try 55 'int main() {int i; int j; i=0; j=0; while(i<10) j = j + (i = i + 1); return j; }'

try 45 'int main() {int i; int j; j=0; for(i=0; i<10; i = i+1) j = j + i; return j; }'
try 45 'int main() {int i; int j; j=0; i=0; for(; i<10; i = i+1) j = j + i; return j; }'

try 55 'int main() {int i; int j; i=0; j=0; while(i<10) { i=i+1;  j = j + i; } return j; }'
try 200 'int main() {int i; i = 0; if( i == 0 ){ i = 100; i = i * 2;  } return i; }'

try 100 'int main() { foo(); return 100; }'
try 100 'int main() {int i; for(i=0; i<5; i=i+1) {foo();} return 100; }'
try 9 'int main() { return 1 + two() * 4; }'

try 3 'int main() { return plus(1,2); }'
try 4 'int main() { return 1 + plus(1,2); }'
try 50 'int main() { return 2 + plus(2,10) * 4; }'

try 11 'int five() { return 5; } int main() { return 2 * five() + 1; }'
try 5 'int add3(int x) { return x+3; } int main() { return add3(2); }'
try 5 'int add(int x,int y) { return x+y; } int main() { return add(2, 3); }'
try 13 'int fib(int i) {if(i<=1) return 1; return fib(i-1) + fib(i-2); } int main() { return fib(6); }'

try 0 'int main() { int *x; return 0; }'
try 0 'int main() { int **x; return 0; }'
try 0 'int main() { int ***x; return 0; }'

<< COMMENTOUT
try 42 'int main() { int *p; p = alloc(42); return *p; }'

try 42 'int main() { int *p; p = alloc(0); *p = 42; return *p; }'

try 42 'int main() { int x; int *p; p = &x; *p = 42; return x; }'
try 0 'int main() { int x; int *p; p = &x; debug(p); debug(p+1); return 0; }'
try 8 'int main() { int *p; p = alloc1(3,5); return *p + *(p+1); }'
try 8 'int main() { int *p; p = alloc1(3,5); return *p + *(1+p); }'
try 9 'int main() { int *p; p = alloc2(2,7); return *p + *(p-1); }'
try 9 'int main() { int *p; p = alloc2(2,7); return *p + *(-1+p); }'
try 2 'int main() { int **p; p = alloc_ptr_ptr(2); return **p; }'
COMMENTOUT

# I found error when same name variable in each functions
try 103 'int add3( int x ) { int a; a = 3; return x + a; } int main() {int a; a = 100; return add3(a); }'

echo OK

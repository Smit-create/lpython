from ltypes import TypeVar, restriction, i32, f32
from numpy import empty

n: i32
n = TypeVar("n")
T = TypeVar('T')

@restriction
def add(x: T, y: T) -> T:
    pass

def g(n: i32, a: T[n], b: T[n]):
  r: T[n]
  r = empty(n)
  i: i32
  for i in range(n):
    r[i] = add(a[i], b[i])
  print(r[0])

def main():
    a_int: i32[1] = empty(1)
    a_int[0] = 400
    b_int: i32[1] = empty(1)
    b_int[0] = 20
    g(1, a_int, b_int)
    a_float: f32[1] = empty(1)
    a_float[0] = 400.0
    b_float: f32[1] = empty(1)
    b_float[0] = 20.0
    g(1, a_float ,b_float)

main()
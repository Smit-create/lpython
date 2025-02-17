from ltypes import i32, f64


def pi() -> f64:
    return 3.141592653589793238462643383279502884197


def e() -> f64:
    return 2.718281828459045235360287471352662497757


def tau() -> f64:
    return 6.283185307179586


def factorial(x: i32) -> i32:
    """
    Computes the factorial of `x`.
    """

    if x < 0:
        return 0
    result: i32
    result = 1
    i: i32
    for i in range(1, x+1):
        result *= i
    return result


def comb(n: i32, k: i32) -> i32:
    """
    Computes the result of `nCk`, i.e, the number of ways to choose `k`
    items from `n` items without repetition and without order.
    """

    if n < k or n < 0:
        return 0
    return factorial(n)//(factorial(k)*factorial(n-k))


def perm(n: i32, k: i32) -> i32:
    """
    Computes the result of `nPk`, i.e, the number of ways to choose `k` items
    from `n` items without repetition and with order.
    """

    if n < k or n < 0:
        return 0
    return factorial(n)//factorial(n-k)


def isqrt(n: i32) -> i32:
    """
    Computes the integer square root of the nonnegative integer `n`.
    """
    if n < 0:
        raise ValueError('`n` should be nonnegative')
    low: i32
    mid: i32
    high: i32
    low = 0
    high = n+1
    while low + 1 < high:
        mid = (low + high)//2
        if mid*mid <= n:
            low = mid
        else:
            high = mid
    return low


def degrees(x: f64) -> f64:
    """
    Convert angle `x` from radians to degrees.
    """
    return x * 180.0 / pi()


def radians(x: f64) -> f64:
    """
    Convert angle `x` from degrees to radians.
    """
    return x * pi() / 180.0


def fabs(x: f64) -> f64:
    """
    Return the absolute value of `x`.
    """
    if x < 0.0:
        return -x
    return x


def ldexp(x: f64, i: i32) -> f64:
    return x * (2**i)


def exp(x: f64) -> f64:
    """
    Return `e` raised to the power `x`.
    """
    return e()**x


def pow(x: f64, y: f64) -> f64:
    """
    Return `x` raised to the power `y`.
    """
    return x**y

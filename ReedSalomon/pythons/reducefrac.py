
def gcd(a, b):
    while b != 0:
        a, b = b, a % b
    return a

def reduce_fraction(numerator, denominator):
    common_divisor = gcd(numerator, denominator)
    return numerator // common_divisor, denominator // common_divisor

# Example usage:
numerator = 42
denominator = 2
reduced_numerator, reduced_denominator = reduce_fraction(numerator, denominator)
print(f"The reduced fraction is {reduced_numerator}/{reduced_denominator}")

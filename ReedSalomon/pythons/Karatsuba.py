def add_polynomials(poly1, poly2):
    length = max(len(poly1), len(poly2))
    result = [0] * length
    for i in range(len(poly1)):
        result[i] += poly1[i]
    for i in range(len(poly2)):
        result[i] += poly2[i]
    return result

def subtract_polynomials(poly1, poly2):
    length = max(len(poly1), len(poly2))
    result = [0] * length
    for i in range(len(poly1)):
        result[i] += poly1[i]
    for i in range(len(poly2)):
        result[i] -= poly2[i]
    return result

def karatsuba(poly1, poly2):
    n = max(len(poly1), len(poly2))
    
    # Base case: if the polynomial is of length 1
    if n == 1:
        return [poly1[0] * poly2[0]]
    
    # Make both polynomials the same length by padding with zeros
    while len(poly1) < n:
        poly1.append(0)
    while len(poly2) < n:
        poly2.append(0)

    # If odd length, pad one more zero to make even length
    if n % 2 != 0:
        poly1.append(0)
        poly2.append(0)
        n += 1
    
    mid = n // 2

    low1 = poly1[:mid]
    high1 = poly1[mid:]
    low2 = poly2[:mid]
    high2 = poly2[mid:]

    z0 = karatsuba(low1, low2)
    z1 = karatsuba(add_polynomials(low1, high1), add_polynomials(low2, high2))
    z2 = karatsuba(high1, high2)

    # Initialize the result array
    result = [0] * (2 * n - 1)
    
    # Combine the results
    for i in range(len(z0)):
        result[i] += z0[i]
    for i in range(len(z1)):
        result[i + mid] += z1[i] - z0[i] - z2[i]
    for i in range(len(z2)):
        result[i + 2 * mid] += z2[i]

    return result

# Example usage:
poly1 = [1, 2, 3]  # Represents 1 + 2x + 3x^2
poly2 = [4, 5, 6]  # Represents 4 + 5x + 6x^2
result = karatsuba(poly1, poly2)
print(result)  # Output should be [4, 13, 28, 27, 18]

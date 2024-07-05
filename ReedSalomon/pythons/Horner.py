def horner(coefs, x):
    result = coefs[0]
    for coef in coefs[1:]:
        result = result * x + coef
    return result

# Example usage
coefs = [2, -6, 2, -1]  # Coefficients of 2x^3 - 6x^2 + 2x - 1
x = 3
print(horner(coefs, x))  # Output should be 5

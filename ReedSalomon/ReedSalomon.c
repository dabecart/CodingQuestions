#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RS_MAX_POLY_DEGREE 10
#define MODULUS 257
#define EXTRA_POINTS 3

/***************************************************************************************************
 * BASIC MATH
 **************************************************************************************************/
int max(int a, int b){
    if(a >= b) return a;
    else return b;
}

int gcd(int a, int b){
    while(b != 0){
        int temp = a;
        a = b;
        b = temp % b;
    }
    return a;
}

int mod(int x){
    while(x < 0)    x += MODULUS;
    return x % MODULUS;
}

/***************************************************************************************************
 * FRACTION
 **************************************************************************************************/
typedef struct{
    int a, b;
} Fraction;

const Fraction ZERO = {0,1};
const Fraction ONE  = {1,1};

Fraction reduceFrac(Fraction x){
    if(x.b == 0){
        printf("Dividing by zero!");
        exit(-1);
    }
    int commonDivisor = gcd(x.a, x.b);
    x = (Fraction) {x.a/commonDivisor, x.b/commonDivisor};
    if(x.b < 0){
        x.b = -x.b;
        x.a = -x.a;
    }
    return x;
}

Fraction sumFrac(Fraction x, Fraction y){
    return reduceFrac((Fraction){x.a*y.b + y.a*x.b, x.b*y.b});
}

Fraction subsFrac(Fraction x, Fraction y){
    return reduceFrac((Fraction){x.a*y.b - y.a*x.b, x.b*y.b});
}

Fraction multFrac(Fraction x, Fraction y){
    return reduceFrac((Fraction){x.a*y.a, x.b*y.b});
}

// Instead of doing it by brute force, use the Extended Euclidean algorithm?
Fraction modFrac(Fraction x){
    if(x.b == 1){
        x.a = mod(x.a);
        return x;
    }

    // The denominator is NOT coprime with the modulus, the inverse does not exist.
    if(x.b % MODULUS == 0){
        x.a = -1;
        x.b = 1;
        return x;
    }

    // The inverse exists and needs to be found:
    // n === 1/b mod p, bn === 1 mod p  <-- Find n!
    int n = 2; // No need to check 1 nor 0.
    for(; n < MODULUS; n++){
        if((x.b*n)%MODULUS == 1) break;
    }

    // The inverse couldn't be found.
    if(n == MODULUS){
        x.a = -1;
        x.b = 1;
        return x;
    }

    x.a = mod(x.a*n);
    x.b = 1;
    return x;
}

int equalFractions(Fraction x, Fraction y){
    x = reduceFrac(x);
    y = reduceFrac(y);
    return (x.a == y.a) && (x.b == y.b);
}

void printFrac(Fraction x){
    if(x.b == 1) printf("%d", x.a);
    else printf("%d/%d", x.a, x.b);
}

/***************************************************************************************************
 * POLYNOMIAL
 **************************************************************************************************/
typedef struct{
    int degree;
    Fraction coeffs[RS_MAX_POLY_DEGREE+1];
} Polynomial;

const Polynomial POLY_ZERO = {
    .degree = 0,
    .coeffs = {(Fraction){0,1}},
};

const Polynomial POLY_ONE = {
    .degree = 0,
    .coeffs = {(Fraction){1,1}},
};

Polynomial createPoly(int* coeffs, int degree){
    Polynomial p = {.degree = degree};
    for(int i = 0; i <= degree; i++){
        p.coeffs[i] = (Fraction){coeffs[i], 1};
    }
    return p;
}

Polynomial createEmptyPoly(int degree){
    Polynomial p = {.degree = degree};
    for(int i = 0; i <= degree; i++){
        p.coeffs[i] = ZERO;
    }
    return p;
}

Polynomial reducePoly(Polynomial p){
    while(p.coeffs[p.degree].a == 0){
        p.degree--;
    }
    return p;
}

Polynomial sumPoly(Polynomial p, Polynomial q){
    Polynomial r = {};
    r.degree = max(p.degree, q.degree);
    for(int i = 0; i <= r.degree; i++){
        r.coeffs[i] = ZERO;
        if(i <= p.degree) r.coeffs[i] = sumFrac(r.coeffs[i], p.coeffs[i]);
        if(i <= q.degree) r.coeffs[i] = sumFrac(r.coeffs[i], q.coeffs[i]);
    }
    return reducePoly(r);
}

Polynomial subsPoly(Polynomial p, Polynomial q){
    Polynomial r = {.degree = max(p.degree, q.degree)};
    for(int i = 0; i <= r.degree; i++){
        r.coeffs[i] = ZERO;
        if(i <= p.degree) r.coeffs[i] = sumFrac(r.coeffs[i], p.coeffs[i]);
        if(i <= q.degree) r.coeffs[i] = subsFrac(r.coeffs[i], q.coeffs[i]);
    }
    return reducePoly(r);
}

// Maybe implement Karatsuba algorithm if necessary?
// This naive approach is O(n^2).
Polynomial multPoly(Polynomial p, Polynomial q){
    Polynomial r = createEmptyPoly(p.degree + q.degree);
    if(r.degree > RS_MAX_POLY_DEGREE){
        printf("Degree overflow\n");
        exit(-1);
    }

    for(int i = 0; i <= p.degree; i++){
        for(int j = 0; j <= q.degree; j++){
            r.coeffs[i+j] = sumFrac(r.coeffs[i+j], multFrac(p.coeffs[i], q.coeffs[j]));
        }
    }
    return reducePoly(r);
}

Polynomial multPolyByFrac(Polynomial p, Fraction a){
    for(int i = 0; i <= p.degree; i++){
        p.coeffs[i] = multFrac(p.coeffs[i], a);
    }
    return reducePoly(p);
}

// Implementation of Horner's Method, O(n) instead of O(n^2).
// p(x) = a + bx + cx^2 + dx^3 = a + x(b + c(x + dx))
Fraction evaluatePoly(Polynomial p, Fraction x){
    Fraction px = p.coeffs[p.degree];
    for(int i = p.degree-1; i >= 0; i--){
        px = sumFrac(p.coeffs[i], multFrac(px, x));
    }
    return px;
}

void printPoly(Polynomial p){
    printFrac(p.coeffs[0]);
    if(p.degree == 0) return;
    
    for(int i = 1; i <= p.degree; i++){
        printf(" + ");
        printFrac(p.coeffs[i]);
        printf("*x");
        if(i != 1) printf("^%d ", i);
    }
}

/***************************************************************************************************
 * LAGRANGIAN INTERPOLATION
 **************************************************************************************************/

/***************************************************************************************************
 * \brief Calculates a function which is zero at all [x] except at [one], where it is [valueAtOne].
 **************************************************************************************************/
Polynomial createSingleLagrangeInterp(int one, int* x, int count, int valueAtOne){
    Polynomial p = POLY_ONE;
    for(int i = 0; i < count; i++){
        // Skip the one.
        if(x[i] == one) continue;

        Polynomial zeroP = {
            .degree = 1,
            .coeffs = {(Fraction){-x[i], 1},  ONE},
        };
        p = multPoly(p, zeroP);
    }
    Fraction pEvalAtOne = evaluatePoly(p, (Fraction){one, 1});
    Fraction pFactor = reduceFrac((Fraction){valueAtOne*pEvalAtOne.b, pEvalAtOne.a});
    p = multPolyByFrac(p, pFactor);
    return p;
}

Polynomial createLagrangeInterp(int* x, int* y, int count){
    Polynomial p = POLY_ZERO;
    for(int i = 0; i < count; i++){
        p = sumPoly(p, createSingleLagrangeInterp(x[i], x, count, y[i]));
    }
    return p;
}

/***************************************************************************************************
 * ERROR CORRECTION ALGORITHM
 **************************************************************************************************/

// 0 if NO error, 1 if corrected and -1 if imposible to correct.
int checkPoints(int* rx, int* ry, int len, int pointsPerLagrange, int* indices){
    int pointsNotOk = 0;
    
    int x[pointsPerLagrange];
    int y[pointsPerLagrange];
    for(int i = 0; i < pointsPerLagrange; i++){
        x[i] = rx[indices[i]];
        printf("%d", x[i]);
        y[i] = ry[indices[i]];
    }
    printf("\n");

    Polynomial p = createLagrangeInterp(x, y, pointsPerLagrange);

    // Get the points from rx that aren't in indices, using the fact that they are ordered from 
    // lesser to greater to reduce computation time from O(n^2) to O(n).
    int i = 0, j = 0;
    while(i < len || j < (pointsPerLagrange-1)){
        if(rx[i] != rx[indices[j]]){
            // Found a value that it's not on indices.
            Fraction eval = evaluatePoly(p, (Fraction){rx[i], 1});
            eval = modFrac(eval);
            if(eval.a == -1){   
                // The result was a fraction that couldn't be inversed, therefore this function is
                // not valid.
                return -1;
            }else{
                pointsNotOk += eval.a != ry[i];
            }
        }else{
            if(j < (pointsPerLagrange-1)) j++;
        }
        i++;
    }

    // There are too much errors to be fixed.
    if(pointsNotOk >= EXTRA_POINTS)    return -1;
    
    // No errors found!
    if(pointsNotOk == 0)               return 0;

    // Errors were found, but can be fixed by evaluating the polynomial.
    for(i = 0; i < len; i++){
        Fraction eval = evaluatePoly(p, (Fraction){rx[i], 1});
        eval = modFrac(eval);
        ry[i] = eval.a;
    }
    return 1;
}

/***************************************************************************************************
 * @brief Creates the combination of all points without repetition.
 * @param rx. Array of x, the points of evaluation of the polynomials.
 * @param ry. Array of y, the received values.
 * @param len. Number of received points.
 * @param pointsPerLagrange. The number of points used to create the first lagragian.
 * @param indices. The indices/points which will be used to create the polynomial.
 * @param indexValue. Which index is currently being written.
 * @param indexPosition. To which position is the previous index value being written to [indices].
 ***************************************************************************************************/
int doCombinations(int* rx, int* ry, int len, int pointsPerLagrange, int* indices, 
                   int indexValue, int indexPosition){
    // If the number of points taken are enough, create the polynomial and check.
    if(indexPosition >= pointsPerLagrange){
        return checkPoints(rx, ry, len, pointsPerLagrange, indices);
    }

    for(int i = indexValue; i < len; i++){
        indices[indexPosition] = rx[i];
        int ret = doCombinations(rx, ry, len, pointsPerLagrange, indices, i + 1, indexPosition + 1);
        // If the combination cannot be checked, continue searching for a new combination.
        // If no error was found or the error was fixed, no need to continue searching.
        if(ret != -1)   return ret;
    }
    // If this is reached, there are more errors than the maximum able for the algorithm to fix.
    return -1;
}

int verifyMessage(int* rx, int* ry, int len, int pointsPerLagrange){
    // Number of combinations is Bi<numberOfPoints, pointsPerCheck>, where Bi<a,b>=a!/b!/(a-b)!
    int indices[pointsPerLagrange];
    return doCombinations(rx, ry, len, pointsPerLagrange, indices, 0, 0);
}

/***************************************************************************************************
 * SIMULATION
 **************************************************************************************************/

int generateRandom(int lower, int upper) {
    if (lower > upper) {
        int temp = lower;
        lower = upper;
        upper = temp;
    }

    // Generate random number within the range [lower, upper]
    int num = (rand() % (upper - lower + 1)) + lower;
    return num;
}

int createSimulation(int minimumPoints, int maxPoints, int maxErrors){
    int numPoints = generateRandom(minimumPoints, maxPoints);
    int numErrors = generateRandom(0, maxErrors);

    int x[numPoints];
    int y[numPoints];

    for(int i = 0; i < numPoints; i++){
        x[i] = i;
        y[i] = generateRandom(0,255);
    }

    Polynomial p = createLagrangeInterp(x, y, numPoints);

    int xx[numPoints + EXTRA_POINTS];
    int yy[numPoints + EXTRA_POINTS];

    for(int i = 0; i < numPoints+EXTRA_POINTS; i++){
        xx[i] = i;

        Fraction result = evaluatePoly(p, (Fraction){xx[i], 1});
        result = modFrac(result);
        
        if(result.a == -1){
            printf("Error modding the following message:\n");
            for(int i = 0; i < numPoints; i++){
                printf("x = %d  ->  y = %d\n", x[i], y[i]);
            }
            exit(-1);
        }
        
        yy[i] = result.a;
    }

    // Up to this point, the points are generated and "sent".
    int errory[numPoints + EXTRA_POINTS];
    memcpy(errory, yy, sizeof(yy));
    
    // New error introduced!
    for(int i = 0; i < numErrors; i++){
        errory[generateRandom(0, numPoints + EXTRA_POINTS - 1)] = generateRandom(0,255);
    }

    int ry[numPoints + EXTRA_POINTS];
    memcpy(ry, errory, sizeof(errory));

    // Find the error.
    int success = verifyMessage(xx, ry, numPoints+EXTRA_POINTS, numPoints);
    if(success != -1){
        int sameAsSent = 1;
        for(int i = 0; i < numPoints+EXTRA_POINTS; i++){
            sameAsSent &= yy[i] == ry[i];
        }
        if(!sameAsSent){
            printf("points: %d\n", numPoints);
            printPoly(p);
            printf("NOT FIXED!\nX : ");
            for(int i = 0; i < numPoints+EXTRA_POINTS; i++){
                printf("%d, ", xx[i]);
            }
            printf("\nY : ");
            for(int i = 0; i < numPoints+EXTRA_POINTS; i++){
                printf("%d, ", yy[i]);
            }
            printf("\nEY: ");
            for(int i = 0; i < numPoints+EXTRA_POINTS; i++){
                printf("%d, ", errory[i]);
            }
            printf("\nRY: ");
            for(int i = 0; i < numPoints+EXTRA_POINTS; i++){
                printf("%d, ", ry[i]);
            }
            printf("\n\n");
        }
        return sameAsSent;
    }
    return 0;
}

void testBench(){
    int success = 0;
    int totalTests = 100;

    for(int i = 0; i < totalTests; i++){
        success += createSimulation(4, RS_MAX_POLY_DEGREE-EXTRA_POINTS, EXTRA_POINTS-1);
    }
    printf("Success: %d/%d\n", success, totalTests);
}

int testCase(){
    int y[] = {177, 96, 222, 177, 128, 173, 8};
    int numPoints = sizeof(y)/sizeof(int);

    int x[numPoints];
    for(int i = 0; i < numPoints; i++){
        x[i] = i;
    }

    Polynomial p = createLagrangeInterp(x, y, numPoints);

    int xx[numPoints + EXTRA_POINTS];
    int yy[numPoints + EXTRA_POINTS];

    for(int i = 0; i < numPoints+EXTRA_POINTS; i++){
        xx[i] = i;

        Fraction result = evaluatePoly(p, (Fraction){xx[i], 1});
        result = modFrac(result);
        
        if(result.a == -1){
            printf("Error modding the following message:\n");
            for(int i = 0; i < numPoints; i++){
                printf("x = %d  ->  y = %d\n", x[i], y[i]);
            }
            exit(-1);
        }
        
        yy[i] = result.a;
    }

    // Up to this point, the points are generated and "sent".
    int errory[numPoints + EXTRA_POINTS];
    memcpy(errory, yy, sizeof(yy));
    
    // New error introduced!
    errory[1] = 65;

    int ry[numPoints + EXTRA_POINTS];
    memcpy(ry, errory, sizeof(yy));

    // Find the error.
    int success = verifyMessage(xx, ry, numPoints+EXTRA_POINTS, numPoints);
    if(success != -1){
        int sameAsSent = 1;
        for(int i = 0; i < numPoints+EXTRA_POINTS; i++){
            sameAsSent &= yy[i] == ry[i];
        }
        if(!sameAsSent){
            printf("points: %d\n", numPoints);
            printPoly(p);
            printf("NOT FIXED!\n");
            for(int i = 0; i < numPoints+EXTRA_POINTS; i++){
                printf("%d, ", xx[i]);
            }
            printf("\nY : ");
            for(int i = 0; i < numPoints+EXTRA_POINTS; i++){
                printf("%d, ", yy[i]);
            }
            printf("\nEY: ");
            for(int i = 0; i < numPoints+EXTRA_POINTS; i++){
                printf("%d, ", errory[i]);
            }
            printf("\nRY: ");
            for(int i = 0; i < numPoints+EXTRA_POINTS; i++){
                printf("%d, ", ry[i]);
            }
            printf("\n\n");
        }
        return sameAsSent;
    }
    return 0;
}

/***************************************************************************************************
 * MAIN
 **************************************************************************************************/
int main(){
    testCase();
}
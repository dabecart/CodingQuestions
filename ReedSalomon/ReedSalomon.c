#include <stdio.h>
#include <stdlib.h>

#define RS_MAX_POLY_DEGREE 10

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

/***************************************************************************************************
 * FRACTION
 **************************************************************************************************/
typedef struct{
    int a, b;
} Fraction;

const Fraction ZERO = {0,1};
const Fraction ONE  = {1,1};

Fraction reduceFrac(Fraction x){
    int commonDivisor = gcd(x.a, x.b);
    return (Fraction) {x.a/commonDivisor, x.b/commonDivisor};
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

Fraction divFrac(Fraction x, Fraction y){
    return reduceFrac((Fraction){x.a*y.b, x.b*y.a});
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

const Polynomial POLY_ONE = {
    .degree = 0,
    .coeffs = {(Fraction){1,1}}
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
 * \brief Calculates a function which is zero at [zeros] and [valueAtOne] in [one].
 **************************************************************************************************/
Polynomial createLagrangeInterp(int one, int* zeros, int zerosCount, int valueAtOne){
    Polynomial p = POLY_ONE;
    for(int i = 0; i < zerosCount; i++){
        Polynomial zeroP = {
            .degree = 1,
            .coeffs = {(Fraction){-zeros[i], 1},  ONE},
        };
        p = multPoly(p, zeroP);
    }
    
    // This should be an integer.
    Fraction pEvalAtOne = evaluatePoly(p, (Fraction){one, 1});
    if(pEvalAtOne.b != 1){
        printf("This values should be an integer\n");
        exit(-1);
    }

    Fraction pFactor = reduceFrac((Fraction){valueAtOne, pEvalAtOne.a});
    p = multPolyByFrac(p, pFactor);
    return p;
}

/***************************************************************************************************
 * MAIN
 **************************************************************************************************/
int main(){
    int a[] = {1,2,3,4};
    int b[] = {1,2};

    Polynomial ap = createPoly(a, sizeof(a)/sizeof(int)-1);
    Polynomial bp = createPoly(b, sizeof(b)/sizeof(int)-1);

    printFrac(evaluatePoly(ap, sumFrac(ONE,ONE)));

}
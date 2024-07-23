/***************************************************************************************************
 * @file ReedSalomon.c
 * @brief The bulk of the Reed Salomon Algorithm.
 *
 * @version   1.0
 * @date      2024-07-23
 * @author    @dabecart
 *
 * @license
 * This project is licensed under the MIT License - see the LICENSE file for details.
 **************************************************************************************************/

#include "ReedSalomon.h"

/***************************************************************************************************
 * BASIC MATH
 **************************************************************************************************/

inline int max(int a, int b){
    if(a >= b) return a;
    else return b;
}

// Calculates the parity of the number of bits of x.
inline int parity(int x){
    int y = x ^ (x >> 1);
    y     = y ^ (y >> 2);
    y     = y ^ (y >> 4);
    y     = y ^ (y >> 8);
    y     = y ^ (y >> 16);
    return y & 1;
}

inline int arrayParity(int* x, int len){
    int par = 0;
    for(int i = 0; i < len; i++){
        par ^= parity(x[i]);
    }
    return par;
}

// CRC-16-CCITT
unsigned short calculateCRC(unsigned char *data, size_t length) {
    const unsigned short POLYNOMIAL  = 0x1021;
    const unsigned short INITIAL_CRC = 0xFFFF;

    unsigned short crc = INITIAL_CRC;
    for (size_t byteIndex = 0; byteIndex < length; byteIndex++) {
        crc ^= (unsigned short)data[byteIndex] << 8;

        for (int bit = 0; bit < 8; bit++) {
            if (crc & 0x8000) {
                crc = (crc << 1) ^ POLYNOMIAL;
            } else {
                crc <<= 1;
            }
        }
    }
    return crc;
}

/***************************************************************************************************
 * MOD INTEGER
 **************************************************************************************************/

const ModInt ZERO = 0;
const ModInt ONE  = 1;

inline ModInt mod(ModInt x){
    return x%MODULUS;
}

inline ModInt sumModInt(ModInt x, ModInt y){
    return mod(x+y);
}

inline ModInt multModInt(ModInt x, ModInt y){
    return mod(x*y);
}

// Calculates the mod of the fraction a/b. For that, this function has to obtain the multiplicative
// inverse of b in mod MODULE.
ModInt modFrac(ModInt a, ModInt b){
    if(b == 1){
        return mod(a);
    }

    // The denominator is NOT coprime with the modulus, the inverse does not exist.
    if(b % MODULUS == 0){
        return -1;
    }

    ModInt n = 0;
#ifdef MOD_USE_NAIVE
    // Modulus of a fraction: Brute force approach.
    // The inverse exists and needs to be found:
    // n === 1/b mod p, bn === 1 mod p  <-- Find n!
    n = 2; // No need to check 1 nor 0.
    for(; n < MODULUS; n++){
        if((b*n)%MODULUS == 1) break;
    }

    // The inverse couldn't be found.
    if(n == MODULUS){
        return -1;
    }
#endif

#ifdef MOD_USE_EUCLID
    // Modulus of a fraction: Extended Euclid algorithm. 10% faster than naive!
    int A = b, M = MODULUS;
    int y = 0, x = 1;
 
    if (M == 1)
        return 0;
 
    while (A > 1) {
        // q is quotient
        int q = A / M;
        int t = M;
 
        // m is remainder now, process same as Euclid's algo
        M = A % M;
        A = t;
        t = y;

        // Update y and x
        y = x - q * y;
        x = t;
    }
 
    // Make x positive
    if (x < 0)  x += MODULUS;
    n = x;
#endif

#ifdef MOD_USE_ARRAY
    // Index 0 is not valid.
    const unsigned char nResults[] = {0, 1, 129, 86, 193, 103, 43, 147, 225, 200, 180, 187, 150, 178, 202, 120, 241, 121, 100, 230, 90, 49, 222, 190, 75, 72, 89, 238, 101, 195, 60, 199, 249, 148, 189, 235, 50, 132, 115, 145, 45, 163, 153, 6, 111, 40, 95, 175, 166, 21, 36, 126, 173, 97, 119, 243, 179, 248, 226, 61, 30, 59, 228, 102, 253, 87, 74, 234, 223, 149, 246, 181, 25, 169, 66, 24, 186, 247, 201, 244, 151, 165, 210, 96, 205, 127, 3, 65, 184, 26, 20, 209, 176, 152, 216, 46, 83, 53, 139, 135, 18, 28, 63, 5, 215, 164, 177, 245, 188, 224, 250, 44, 218, 116, 124, 38, 113, 134, 159, 54, 15, 17, 158, 140, 114, 220, 51, 85, 255, 2, 172, 206, 37, 143, 117, 99, 240, 242, 203, 98, 123, 144, 219, 133, 141, 39, 213, 7, 33, 69, 12, 80, 93, 42, 252, 194, 229, 239, 122, 118, 204, 174, 211, 41, 105, 81, 48, 237, 231, 73, 192, 254, 130, 52, 161, 47, 92, 106, 13, 56, 10, 71, 233, 191, 88, 232, 76, 11, 108, 34, 23, 183, 170, 4, 155, 29, 198, 227, 196, 31, 9, 78, 14, 138, 160, 84, 131, 221, 236, 91, 82, 162, 217, 146, 251, 104, 94, 212, 112, 142, 125, 207, 22, 68, 109, 8, 58, 197, 62, 156, 19, 168, 185, 182, 67, 35, 208, 167, 27, 157, 136, 16, 137, 55, 79, 107, 70, 77, 57, 32, 110, 214, 154, 64, 171, 128, 48};
    n = nResults[b];
#endif
    return multModInt(a,n);
}

/***************************************************************************************************
 * POLYNOMIAL
 **************************************************************************************************/

typedef struct{
    int degree;
    ModInt coeffs[RS_MAX_POLY_DEGREE+1];
} Polynomial;

const Polynomial POLY_ZERO = {
    .degree = 0,
    .coeffs = {0},
};

const Polynomial POLY_ONE = {
    .degree = 0,
    .coeffs = {1},
};

void createPoly(int* coeffs, int degree, Polynomial* pout){
    pout->degree = degree;
    for(int i = 0; i <= degree; i++){
        pout->coeffs[i] = coeffs[i];
    }
}

void createEmptyPoly(int degree, Polynomial* pout){
    pout->degree = degree;
    for(int i = 0; i <= degree; i++){ 
        pout->coeffs[i] = ZERO;
    }
}

void reducePoly(Polynomial* p){
    while(p->coeffs[p->degree] == 0 && p->degree > 0){
        p->degree--;
    }
}

inline void sumPoly(Polynomial* p, Polynomial* q, Polynomial* pout){
    Polynomial newP;
    if(p == pout){
        memcpy(&newP, p, sizeof(Polynomial));
        p = &newP;
        *pout = POLY_ZERO;
    }

    pout->degree = max(p->degree, q->degree);
    for(int i = 0; i <= pout->degree; i++){
        pout->coeffs[i] = ZERO;
        if(i <= p->degree) pout->coeffs[i] = sumModInt(pout->coeffs[i], p->coeffs[i]);
        if(i <= q->degree) pout->coeffs[i] = sumModInt(pout->coeffs[i], q->coeffs[i]);
    }
    reducePoly(pout);
}

// This naive approach is O(n^2).
inline void multPoly(Polynomial* p, Polynomial* q, Polynomial* pout){
    Polynomial newP;
    if(p == pout){
        memcpy(&newP, p, sizeof(Polynomial));
        p = &newP;
        *pout = POLY_ZERO;
    }

    pout->degree = p->degree + q->degree;
    if(pout->degree > RS_MAX_POLY_DEGREE){
        printf("Degree overflow\n");
        exit(-1);
    }

    for(int i = 0; i <= p->degree; i++){
        for(int j = 0; j <= q->degree; j++){
            pout->coeffs[i+j] = sumModInt(
                                    pout->coeffs[i+j], 
                                    multModInt(p->coeffs[i], q->coeffs[j])
                                );
        }
    }
    reducePoly(pout);
}

inline void multPolyByFrac(Polynomial* p, ModInt a, Polynomial* pout){
    Polynomial polyA = {
        .degree = 0,
        .coeffs = {a},
    };
    multPoly(p, &polyA, pout);
}

// Implementation of Horner's Method, O(n) instead of O(n^2).
// p(x) = a + bx + cx^2 + dx^3 = a + x(b + c(x + dx))
ModInt evaluatePoly(Polynomial* p, ModInt x){
    ModInt px = p->coeffs[p->degree];
    for(int i = p->degree-1; i >= 0; i--){
        px = sumModInt(p->coeffs[i], multModInt(px, x));
    }
    return px;
}

void printPoly(Polynomial* p){
    printf("%d", p->coeffs[0]);
    if(p->degree == 0) return;
    
    for(int i = 1; i <= p->degree; i++){
        printf(" + ");
        printf("%d", p->coeffs[i]);
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
inline void createSingleLagrangeInterp(int one, int* x, int count, int valueAtOne, Polynomial* pout){
    *pout = POLY_ONE;
    Polynomial zeroP = {
        .degree = 1,
    };

    for(int i = 0; i < count; i++){
        // Skip the one.
        if(x[i] == one) continue;

        // Add a zero to the polynomial: (x - xi) mod MODULUS === (x + MODULUS - xi) mod MODULUS,
        // supposing xi >= -MODULUS. This allows us to use unsigned short as ModInt in mod 257!
        zeroP.coeffs[0] = MODULUS-x[i]; 
        zeroP.coeffs[1] = ONE;

        multPoly(pout, &zeroP, pout);
    }

    ModInt pFactor = modFrac(valueAtOne, evaluatePoly(pout, one));
    multPolyByFrac(pout, pFactor, pout);
}

void createLagrangeInterp(int* x, int* y, int count, Polynomial* pout){
    *pout = POLY_ZERO;
    Polynomial singleLag;
    for(int i = 0; i < count; i++){
        createSingleLagrangeInterp(x[i], x, count, y[i], &singleLag);
        sumPoly(pout, &singleLag, pout);
    }
}

/***************************************************************************************************
 * HAMMING CODE (with whole numbers)
 **************************************************************************************************/
int calculateHamming(int* x, int* y, int len){
    int hamming = 0;
    for(int i = 0; i < len; i++){
        if(parity(y[i])){
            hamming ^= x[i];
        }        
    } 
    return hamming;
}

/***************************************************************************************************
 * ERROR CORRECTION ALGORITHM
 **************************************************************************************************/

// 0 if NO error, 1 if corrected and -1 if impossible to correct.
AlgorithmReturn checkPoints(int* rx, int* ry, int len, int pointsPerLagrange, int* indices){
    int pointsNotOk = 0;
    
    int x[pointsPerLagrange];
    int y[pointsPerLagrange];
    for(int i = 0; i < pointsPerLagrange; i++){
        x[i] = rx[indices[i]];
        y[i] = ry[indices[i]];
        // printf("%d,", x[i]);
    }
    // printf("\n");

    Polynomial p;
    createLagrangeInterp(x, y, pointsPerLagrange, &p);

    // Get the points from rx that aren't in indices, using the fact that they are ordered from 
    // lesser to greater to reduce computation time from O(n^2) to O(n).
    int i = 0, j = 0;
    while(i < len || j < (pointsPerLagrange-1)){
        if(rx[i] != rx[indices[j]]){
            // Found a value that it's not on indices.
            ModInt eval = evaluatePoly(&p, rx[i]);
            if(eval == -1){   
                // The result was a fraction that couldn't be inverted, therefore this function is
                // not valid.
                return COULDNT_BE_FIXED;
            }else{
                int pointNotOK = eval != ry[i];
                
                // If the EEPROM is OK and this comparator is saying that a point in EEPROM is wrong
                // skip it, as this function isn't correct.
                if(EEPROM_NOT_CORRUPTED && pointNotOK && i >= (len - EXTRA_POINTS)){
                    return COULDNT_BE_FIXED;
                }
                pointsNotOk += pointNotOK; 
            }
        }else{
            if(j < (pointsPerLagrange-1)) j++;
        }
        i++;
    }

    // There are too much errors to be fixed.
    if(pointsNotOk >= EXTRA_POINTS)    return COULDNT_BE_FIXED;
    
    // No errors found!
    if(pointsNotOk == 0)               return WITHOUT_ERRORS;

    int tempSave[len];
    memcpy(tempSave, ry, len*sizeof(int));

    // Errors were found, but can be fixed by evaluating the polynomial.
    for(i = 0; i < len; i++){
        ry[i] = evaluatePoly(&p, rx[i]);
    }

    // If the EEPROM isn't corrupted, use the Hamming and CRC to double verify.
    if(EEPROM_NOT_CORRUPTED){
        // Check the Hamming. The Hamming is sent after the EXTRA_POINTS in the array ry.
        // If the message is the same, when XORing the Hamming, it should return 0.
        int newHamming = calculateHamming(rx,ry, len);
        int crc = calculateCRC((unsigned char*) ry, len*sizeof(int)) & 0xF0;
        if((newHamming | crc) != ry[len]){
            // Hamming wasn't correct, restore the points.
            memcpy(ry, tempSave, len*sizeof(int));
            return COULDNT_BE_FIXED;
        }
    }

    return FIXED_OK;
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
 * @param hammingBehaviour. If 0, nevermind the Hamming. If 1, only run the points which contain 
 * the Hamming. If -1, only run the points which don't contain the Hamming.
 * @param hammingValue. The value of the Hamming, aka. on which position the wrong point is.
 * @param hammingIndex. Where in [indices] is the Hamming.
 **************************************************************************************************/
AlgorithmReturn doCombinations(int* rx, int* ry, int len, int pointsPerLagrange, 
                   int* indices, int indexValue, int indexPosition,
                   int hammingBehaviour, int hammingValue, int hammingIndex){
    // If the number of points taken are enough, create the polynomial and check.
    if(indexPosition >= pointsPerLagrange){
        if((hammingBehaviour == 0) || (hammingBehaviour == -1) || 
           (hammingBehaviour ==  1 && (indices[hammingIndex] == hammingValue))){
            return checkPoints(rx, ry, len, pointsPerLagrange, indices);
        }
        return COULDNT_BE_FIXED;
    }

    for(int i = indexValue; i < len; i++){
        indices[indexPosition] = rx[i];

        if(i == hammingValue){
            if(hammingBehaviour == -1)  continue;
            hammingIndex = i;
        }

        int nextIndex = i + 1;
        // If we know the EEPROM is working, then we should always try to use the extra points 
        // stored in it. So, when we have pointsPerLagrange-EXTRA_POINTS points from the actual 
        // data, use the EEPROM points.
        if(EEPROM_NOT_CORRUPTED && (indexPosition+1) == (pointsPerLagrange-EXTRA_POINTS)){
            nextIndex = len - EXTRA_POINTS;
            // If the next index is the current one, no more checks to do.
            if(i >= nextIndex){
                break;
            }
        }

        AlgorithmReturn ret = doCombinations(rx, ry, len, pointsPerLagrange, 
                                 indices, nextIndex, indexPosition + 1,
                                 hammingBehaviour, hammingValue, hammingIndex);
        // If the combination cannot be checked, continue searching for a new combination.
        // If no error was found or the error was fixed, no need to continue searching.
        if(ret != COULDNT_BE_FIXED)   return ret;
    }
    // If this is reached, there are more errors than the maximum able for the algorithm to fix.
    return COULDNT_BE_FIXED;
}

AlgorithmReturn verifyMessage(int* rx, int* ry, int len, int pointsPerLagrange){
    // Using Bi<a,b>=a!/b!/(a-b)! ...
    // Number of combinations when EEPROM is faulty: 
    //    > Bi<len, pointsPerLagrange> 
    // Number of combinations when EEPROM is OK: 
    //    > Bi<len - EXTRA_POINTS, pointsPerLagrange - EXTRA_POINTS>
    // Example:
    //    > Using 10 points, with 3 EXTRA_POINTS: Faulty: 286 checks, Non faulty: 120 checks (~42%).
    int indices[pointsPerLagrange];
    AlgorithmReturn verificationStatus = UNDEFINED;
    
    if(EEPROM_NOT_CORRUPTED){
        // If the EEPROM is right, we can use the Hamming code to indicate which point to SKIP in 
        // the case there's ONLY ONE ERROR (which should be the most common case if the 
        // interlacing works ok). 

        // If the error couldn't be fixed, then it must have been that there is more than one error 
        // on the message, so only execute the points which CONTAINS the Hamming.
        int currentHamming = calculateHamming(rx,ry,len) ^ (ry[len] & 0x0F);
        
        // When using the EEPROM the Hamming has to be on the data side, not on the EEPROM side. 
        // If the Hamming is on the EEPROM, that means that there are more than two errors.
        // The Hamming's value is the point on which an error occurred. If there are are more than a
        // single error then the Hamming cannot be trusted.
        if(currentHamming < (len - EXTRA_POINTS)){
            verificationStatus = 
                        doCombinations(rx, ry, len, pointsPerLagrange,
                                    indices, 0, 0, 
                                    -1, currentHamming, 0);       // Skip the Hamming.
            if(verificationStatus < 0){
                verificationStatus =  
                        doCombinations(rx, ry, len, pointsPerLagrange, 
                                    indices, 0, 0, 
                                    1, currentHamming, 0); // Only run Hamming's combinations.
            }
        }else{
            goto dontUseHamming;
        }
    }else{
        dontUseHamming:        
        // Never mind the Hamming.
        verificationStatus = doCombinations(rx, ry, len, pointsPerLagrange, indices, 0, 0, 0, 0, 0);
    }


    // If the verification failed, check if some of the extra points could be points trimmed that
    // exceeded the 255 value set by the byte limit. Remember that extra points are in [0, MODULUS).
    // Example: 256 trimmed as a byte would be 0, so a 0 on the extra points could be 0 or a 256 too
    // if MODULUS was greater than or equal to 255!
    for(int i = len-EXTRA_POINTS; (verificationStatus < 0) && (i < len); i++){
        while((verificationStatus < 0) && (ry[i]+256 < MODULUS)){
            ry[i] += 256;
            // Do it recursively to try all possible combinations.
            verificationStatus = verifyMessage(rx, ry, len, pointsPerLagrange);
        }
    }

    return verificationStatus;
}

// Adds the correction fields at the end of the array (EXTRA_POINTS + 1).
void addErrorCorrectionFields(int* x, int* y, int numPoints, int* xx, int* yy){
    if(x == NULL || y == NULL){
        perror("Input data is NULL!");
        exit(-1);
    }

    Polynomial p;
    createLagrangeInterp(x, y, numPoints, &p);

    // TODO: Modify this so it starts from numPoints instead of 0.
    for(int i = 0; i < numPoints+EXTRA_POINTS; i++){
        xx[i] = i;

        yy[i] = evaluatePoly(&p, xx[i]);
        
        if(yy[i] == -1){
            printf("Error modding the following message:\n");
            for(int i = 0; i < numPoints; i++){
                printf("x = %d  ->  y = %d\n", x[i], y[i]);
            }
            exit(-1);
        }
    }

    // Add Hamming code.
    yy[numPoints + EXTRA_POINTS] = calculateHamming(xx, yy, numPoints + EXTRA_POINTS);
    if(yy[numPoints + EXTRA_POINTS] >= 16){
        printf("Hamming out of bounds\n");
        exit(-1);
    }

    // Add CRC.
    int crc = calculateCRC((unsigned char*)yy, (numPoints+EXTRA_POINTS)*sizeof(int)) & 0xF0;
    yy[numPoints + EXTRA_POINTS] |= crc;
}
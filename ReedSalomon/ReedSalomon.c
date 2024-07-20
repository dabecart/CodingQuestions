#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// The maximum degree of polynomials in this program.
#define RS_MAX_POLY_DEGREE 13
// Number of extra points to evaluate on the polynomial. The number of errors that can be fixed are
// up to EXTRA_POINTS-1. Errors will be detected for up to EXTRA_POINTS, but they will not be 
// fixable. 
#define EXTRA_POINTS 3
// Value for the loading bar.
#define BAR_WIDTH 50
// The modulus we'll be working on. This has to be a PRIME!!!
#define MODULUS 257

// Used for the input values. If inputs are bytes, this value will be 255. 
// Used only for simulations!
#define MAX_DATA_VALUE 255

// If we take that the extra points are not corrupted, the computations are easier and faster to do.
// When the EEPROM is corrupted, we don't take into account the Hamming and CRC codes, as they are 
// redundant checks used to fix >2 bit length errors.
int EEPROM_NOT_CORRUPTED = 1;

/***************************************************************************************************
 * ALGORITHM SELECTION
 **************************************************************************************************/
// #define MOD_USE_NAIVE
#define MOD_USE_EUCLID
// #define MOD_USE_ARRAY

/***************************************************************************************************
 * BASIC MATH
 **************************************************************************************************/
int max(int a, int b){
    if(a >= b) return a;
    else return b;
}

int parity(int x){
    int y = x ^ (x >> 1);
    y     = y ^ (y >> 2);
    y     = y ^ (y >> 4);
    y     = y ^ (y >> 8);
    y     = y ^ (y >> 16);
    return y & 1;
}

int arrayParity(int* x, int len){
    int par = 0;
    for(int i = 0; i < len; i++){
        par ^= parity(x[i]);
    }
    return par;
}

#define POLYNOMIAL 0x1021
#define INITIAL_CRC 0xFFFF

unsigned short calculateCRC(unsigned char *data, size_t length) {
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
typedef int ModInt;

const ModInt ZERO = 0;
const ModInt ONE  = 1;

ModInt mod(ModInt x){
    if(x < 0){
        while(x < 0)    x += MODULUS;
    }else if(x >= MODULUS){
        x %= MODULUS;
    }
    return x;
}

ModInt modFrac(ModInt a, ModInt b){
    if(b == 1){
        return mod(a);
    }

    // The denominator is NOT coprime with the modulus, the inverse does not exist.
    if(b % MODULUS == 0){
        return -1;
    }

    int n = 0;
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

#ifdef MOD_USE_EUCLID
    const unsigned char nResults[] = {0, 1, 129, 86, 193, 103, 43, 147, 225, 200, 180, 187, 150, 178, 202, 120, 241, 121, 100, 230, 90, 49, 222, 190, 75, 72, 89, 238, 101, 195, 60, 199, 249, 148, 189, 235, 50, 132, 115, 145, 45, 163, 153, 6, 111, 40, 95, 175, 166, 21, 36, 126, 173, 97, 119, 243, 179, 248, 226, 61, 30, 59, 228, 102, 253, 87, 74, 234, 223, 149, 246, 181, 25, 169, 66, 24, 186, 247, 201, 244, 151, 165, 210, 96, 205, 127, 3, 65, 184, 26, 20, 209, 176, 152, 216, 46, 83, 53, 139, 135, 18, 28, 63, 5, 215, 164, 177, 245, 188, 224, 250, 44, 218, 116, 124, 38, 113, 134, 159, 54, 15, 17, 158, 140, 114, 220, 51, 85, 255, 2, 172, 206, 37, 143, 117, 99, 240, 242, 203, 98, 123, 144, 219, 133, 141, 39, 213, 7, 33, 69, 12, 80, 93, 42, 252, 194, 229, 239, 122, 118, 204, 174, 211, 41, 105, 81, 48, 237, 231, 73, 192, 254, 130, 52, 161, 47, 92, 106, 13, 56, 10, 71, 233, 191, 88, 232, 76, 11, 108, 34, 23, 183, 170, 4, 155, 29, 198, 227, 196, 31, 9, 78, 14, 138, 160, 84, 131, 221, 236, 91, 82, 162, 217, 146, 251, 104, 94, 212, 112, 142, 125, 207, 22, 68, 109, 8, 58, 197, 62, 156, 19, 168, 185, 182, 67, 35, 208, 167, 27, 157, 136, 16, 137, 55, 79, 107, 70, 77, 57, 32, 110, 214, 154, 64, 171, 128};
    n = nResults[b];
#endif

    return mod(a*n);
}


ModInt sumFrac(ModInt x, ModInt y){
    return mod(x+y);
}

ModInt subsFrac(ModInt x, ModInt y){
    return mod(x-y);
}

ModInt multFrac(ModInt x, ModInt y){
    return mod(x*y);
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

Polynomial createPoly(int* coeffs, int degree){
    Polynomial p = {.degree = degree};
    for(int i = 0; i <= degree; i++){
        p.coeffs[i] = coeffs[i];
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
    while(p.coeffs[p.degree] == 0 && p.degree > 0){
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

Polynomial multPolyByFrac(Polynomial p, ModInt a){
    return multPoly(p, createPoly(&a, 0));
}

// Implementation of Horner's Method, O(n) instead of O(n^2).
// p(x) = a + bx + cx^2 + dx^3 = a + x(b + c(x + dx))
ModInt evaluatePoly(Polynomial p, ModInt x){
    ModInt px = p.coeffs[p.degree];
    for(int i = p.degree-1; i >= 0; i--){
        px = sumFrac(p.coeffs[i], multFrac(px, x));
    }
    return px;
}

void printPoly(Polynomial p){
    printf("%d", p.coeffs[0]);
    if(p.degree == 0) return;
    
    for(int i = 1; i <= p.degree; i++){
        printf(" + ");
        printf("%d", p.coeffs[i]);
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
            .coeffs = {-x[i],  ONE},
        };
        p = multPoly(p, zeroP);
    }
    ModInt pEvalAtOne = evaluatePoly(p, one);
    ModInt pFactor = modFrac(valueAtOne, pEvalAtOne);
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
int checkPoints(int* rx, int* ry, int len, int pointsPerLagrange, int* indices){
    int pointsNotOk = 0;
    
    int x[pointsPerLagrange];
    int y[pointsPerLagrange];
    for(int i = 0; i < pointsPerLagrange; i++){
        x[i] = rx[indices[i]];
        y[i] = ry[indices[i]];
        // printf("%d,", x[i]);
    }
    // printf("\n");

    Polynomial p = createLagrangeInterp(x, y, pointsPerLagrange);

    // Get the points from rx that aren't in indices, using the fact that they are ordered from 
    // lesser to greater to reduce computation time from O(n^2) to O(n).
    int i = 0, j = 0;
    while(i < len || j < (pointsPerLagrange-1)){
        if(rx[i] != rx[indices[j]]){
            // Found a value that it's not on indices.
            ModInt eval = evaluatePoly(p, rx[i]);
            if(eval == -1){   
                // The result was a fraction that couldn't be inverted, therefore this function is
                // not valid.
                return -1;
            }else{
                int pointNotOK = eval != ry[i];
                
                // If the EEPROM is OK and this comparator is saying that a point in EEPROM is wrong
                // skip it, as this function isn't correct.
                if(EEPROM_NOT_CORRUPTED && pointNotOK && i >= (len - EXTRA_POINTS)){
                    return -1;
                }
                pointsNotOk += pointNotOK; 
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

    int tempSave[len];
    memcpy(tempSave, ry, len*sizeof(int));

    // Errors were found, but can be fixed by evaluating the polynomial.
    for(i = 0; i < len; i++){
        ry[i] = evaluatePoly(p, rx[i]);
    }

    // If the EEPROM isn't corrupted, use the Hamming and CRC.
    if(EEPROM_NOT_CORRUPTED){
        // Check the Hamming. The Hamming is sent after the EXTRA_POINTS in the array ry.
        // If the message is the same, when XORing the Hamming, it should return 0.
        int newHamming = calculateHamming(rx,ry, len);
        int crc = calculateCRC((unsigned char*) ry, len*sizeof(int)) & 0xF0;
        if((newHamming | crc) == ry[len]){
            return 1;
        }

        // Restore the points, Hamming wasn't correct.
        memcpy(ry, tempSave, len*sizeof(int));
        return -1;
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
 * @param hammingBehaviour. If 0, nevermind the Hamming. If 1, only run the points which contain 
 * the Hamming. If -1, only run the points which don't contain the Hamming.
 * @param hammingValue. The value of the Hamming, aka. on which position the wrong point is.
 * @param hammingIndex. Where in [indices] is the Hamming.
 **************************************************************************************************/
int doCombinations(int* rx, int* ry, int len, int pointsPerLagrange, 
                   int* indices, int indexValue, int indexPosition,
                   int hammingBehaviour, int hammingValue, int hammingIndex){
    // If the number of points taken are enough, create the polynomial and check.
    if(indexPosition >= pointsPerLagrange){
        if((hammingBehaviour == 0) || (hammingBehaviour == -1) || 
           (hammingBehaviour ==  1 && (indices[hammingIndex] == hammingValue))){
            return checkPoints(rx, ry, len, pointsPerLagrange, indices);
        }
        return -1;
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

        int ret = doCombinations(rx, ry, len, pointsPerLagrange, 
                                 indices, nextIndex, indexPosition + 1,
                                 hammingBehaviour, hammingValue, hammingIndex);
        // If the combination cannot be checked, continue searching for a new combination.
        // If no error was found or the error was fixed, no need to continue searching.
        if(ret != -1)   return ret;
    }
    // If this is reached, there are more errors than the maximum able for the algorithm to fix.
    return -1;
}

int verifyMessage(int* rx, int* ry, int len, int pointsPerLagrange){
    // Using Bi<a,b>=a!/b!/(a-b)! ...
    // Number of combinations when EEPROM is faulty: 
    //    > Bi<len, pointsPerLagrange> 
    // Number of combinations when EEPROM is OK: 
    //    > Bi<len - EXTRA_POINTS, pointsPerLagrange - EXTRA_POINTS>
    // Example:
    //    > Using 10 points, with 3 EXTRA_POINTS: Faulty: 286 checks, Non faulty: 120 checks (~42%).
    int indices[pointsPerLagrange];
    int verificationStatus = 0;
    
    if(EEPROM_NOT_CORRUPTED){
        // If the EEPROM is right, we can use the Hamming code to indicate which point to SKIP in 
        // the case there's ONLY ONE ERROR (which should be the most common case if the 
        // interlacing works ok). 

        // If the error couldn't be fixed, then it must have been that there is more than one error 
        // on the message, so only execute the points which CONTAINS the Hamming.
        int currentHamming = calculateHamming(rx,ry,len) ^ (ry[len] & 0x0F);
        
        // When using the EEPROM the Hamming has to be on the data side, not on the EEPROM side. 
        // If the Hamming is on the EEPROM, that means that there are more than two errors.
        if(currentHamming < len - EXTRA_POINTS){
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
        }
    }else{
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
            verificationStatus = verifyMessage(rx, ry, len, pointsPerLagrange);
        }
    }

    return verificationStatus;
}

void addErrorCorrectionFields(int* x, int* y, int numPoints, int* xx, int* yy){
    if(x == NULL || y == NULL){
        perror("Input data is NULL!");
        exit(-1);
    }

    Polynomial p = createLagrangeInterp(x, y, numPoints);

    for(int i = 0; i < numPoints+EXTRA_POINTS; i++){
        xx[i] = i;

        yy[i] = evaluatePoly(p, xx[i]);
        
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

/***************************************************************************************************
 * RANDOM SIMULATION
 **************************************************************************************************/

int generateRandom(int lower, int upper) {
    if(lower == upper) return lower;

    if (lower > upper) {
        int temp = lower;
        lower = upper;
        upper = temp;
    }

    // Generate random number within the range [lower, upper]
    int num = (rand() % (upper - lower + 1)) + lower;
    return num;
}

int runSimulation(int* x, int* y, int numPoints, int* errX, int* errY, int numErrors){
    int xx[numPoints + EXTRA_POINTS];
    int yy[numPoints + EXTRA_POINTS + 1];

    // Add to the message the error correction fields.
    addErrorCorrectionFields(x, y, numPoints, xx, yy);

    // Up to this point, the points are generated and "sent".
    int errory[numPoints + EXTRA_POINTS + 1];
    memcpy(errory, yy, sizeof(yy));
    
    // New errors are introduced!
    for(int i = 0; i < numErrors; i++){
        if(EEPROM_NOT_CORRUPTED && i >= numPoints){
            printf("If EEPROM is not corrupted, there cannot be errors and the EEPROM side.\n");
            exit(-1);
        }
        errory[errX[i]] = errY[i];
    }

    int ry[numPoints + EXTRA_POINTS + 1];
    memcpy(ry, errory, sizeof(errory));

    // Find the error.
    int success = verifyMessage(xx, ry, numPoints+EXTRA_POINTS, numPoints);
    if(success != -1){
        int sameAsSent = 1;
        for(int i = 0; i < numPoints+EXTRA_POINTS; i++){
            sameAsSent &= yy[i] == ry[i];
        }
        if(!sameAsSent){
            printf("NOT FIXED!\n");
            success = -2;
        }
    }

    if(success < 0){
        printf("Points: %d. Errors: %d\n", numPoints, numErrors);
        printf("\nX :\t");
        for(int i = 0; i < numPoints+EXTRA_POINTS; i++){
            printf("%d,\t", xx[i]);
        }
        printf("\nY :\t");
        for(int i = 0; i < numPoints+EXTRA_POINTS; i++){
            printf("%d,\t", yy[i]);
        }
        printf("\nEY:\t");
        for(int i = 0; i < numPoints+EXTRA_POINTS; i++){
            if(yy[i] == errory[i]){
                printf("\x1B[32m%d\x1B[0m,\t", errory[i]);
            }else{
                printf("\x1B[31m%d\x1B[0m,\t", errory[i]);
            }
        }
        printf("\nRY:\t");
        for(int i = 0; i < numPoints+EXTRA_POINTS; i++){
            if(yy[i] == ry[i]){
                printf("\x1B[32m%d\x1B[0m,\t", ry[i]);
            }else{
                printf("\x1B[31m%d\x1B[0m,\t", ry[i]);
            }
        }
        printf("\n\n");
    }

    return success;
}

void swap(int *a, int *b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

// Function to shuffle an array using Fisher-Yates algorithm to generate the indices for the random
// errors without them being repeated.
void shuffleArray(int *array, int len) {
    for (int i = len - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        swap(&array[i], &array[j]);
    }
}

int createSimulation(int numPoints, int minErrors, int maxErrors){
    int numErrors = generateRandom(minErrors, maxErrors);

    int x[numPoints];
    int y[numPoints];

    for(int i = 0; i < numPoints; i++){
        x[i] = i;
        y[i] = generateRandom(0,MAX_DATA_VALUE);
    }

    int errX[maxErrors];
    int errY[maxErrors];

    // New errors introduced!
    // Generate the Xs (where the errors will happen).
    if(numErrors >= 2){
        int randX[numPoints+EXTRA_POINTS];
        if(EEPROM_NOT_CORRUPTED){
            for(int i = 0; i < numPoints; i++) randX[i] = i;
            shuffleArray(randX, numPoints);
        }else{
            for(int i = 0; i < numPoints+EXTRA_POINTS; i++) randX[i] = i;
            shuffleArray(randX, numPoints+EXTRA_POINTS);
        }
        memcpy(errX, randX, sizeof(int)*numErrors);
    }else{
        for(int i = 0; i < numErrors; i++){
            if(EEPROM_NOT_CORRUPTED){
                errX[i] = generateRandom(0, numPoints - 1);
            }else{
                errX[i] = generateRandom(0, numPoints + EXTRA_POINTS - 1);
            }
        }
    }

    // Generate the Ys (the values of the errors).
    for(int i = 0; i < numErrors; i++){
        // Generate random numbers until the generated number is different from the number in the 
        // array.
        int rnd = 0;
        do{
            rnd = generateRandom(0, 255);
        }while(rnd == y[errX[i]]);
        errY[i] = rnd;
    }

    return runSimulation(x, y, numPoints, errX, errY, numErrors);
}

void printLoadingBar(long progress, long total) {
    int barLen = (progress * BAR_WIDTH) / total;
    printf("Progress: [");
    for (int i = 0; i < BAR_WIDTH; ++i) {
        if (i < barLen) {
            printf("#");
        } else {
            printf(" ");
        }
    }
    printf("] %ld%%: %ld/%ld\r", (progress * 100) / total, progress, total);
    fflush(stdout);
}

void testBench(int totalTests){
    srand(time(0));
    printf("Points per sample: %d\n", RS_MAX_POLY_DEGREE-EXTRA_POINTS);
    printf("Number of errors: rand(%d, %d)\n", 1, EXTRA_POINTS);
    printf("################ TEST BEGIN ################\n");
    int success = 0;
    int noErrors = 0;
    int fixedIncorrectly = 0;

    struct timespec t0, t1;
    long long maxElapsed = 0;
    long long minElapsed = 0x7fffffffffffffffLL;
    long long averageElapsed = 0;

    for(int i = 0; i < totalTests; i++){
        if (clock_gettime(CLOCK_REALTIME, &t0) != 0) {
            perror("clock_gettime");
            return;
        }

        // Run the simulation.
        int result = createSimulation(RS_MAX_POLY_DEGREE-EXTRA_POINTS, 1, EXTRA_POINTS-1);
        success += result >= 0;
        noErrors += result == 0;
        fixedIncorrectly += result == -2;
        
        if (clock_gettime(CLOCK_REALTIME, &t1) != 0) {
            perror("clock_gettime");
            return;
        }
        long long elapsed = 
            (long long)(t1.tv_sec * 1000000000LL + t1.tv_nsec) - 
            (long long)(t0.tv_sec * 1000000000LL + t0.tv_nsec);
        averageElapsed += elapsed;
        if(elapsed > maxElapsed){
            maxElapsed = elapsed;
        }
        if(elapsed < minElapsed){
            minElapsed = elapsed;
        }

        printLoadingBar(i+1, totalTests);
    }
    printf("\n############# TEST RESULTS ################\n");
    printf("Success rate: %d/%d. No errors: %d. Fixed incorrectly: %d.\n", 
           success, totalTests, noErrors, fixedIncorrectly);
    double bitRate = ((double)(RS_MAX_POLY_DEGREE-EXTRA_POINTS))*totalTests*1e9/averageElapsed;
    double byteRate = bitRate / 8.0;
    printf("Bitrate: %0.2f bits/sec. Byterate: %0.2f bytes/sec.\n", bitRate, byteRate);
    printf("Average elapsed time: %lld ns\n", averageElapsed/totalTests);
    printf("Minimum elapsed time: %lld ns\n", minElapsed);
    printf("Maximum elapsed time: %lld ns\n", maxElapsed);
}

/***************************************************************************************************
 * CUSTOM SIMULATION
 **************************************************************************************************/

int testCase(){
    int y[] = { 29,     18,     248,    166};
    int numPoints = sizeof(y)/sizeof(int);

    int x[numPoints];
    for(int i = 0; i < numPoints; i++){
        x[i] = i;
    }

    int errX[] = {1};
    int errY[] = {76};
    int numErrors = sizeof(errX)/sizeof(int);

    return runSimulation(x, y, numPoints, errX, errY, numErrors);
}

/***************************************************************************************************
 * FILE REPARATION
 **************************************************************************************************/

void createRecuperationFile(const char* filename){
    FILE* inputFile = fopen(filename, "rb");
    if (inputFile == NULL) {
        perror("Error opening input file");
        exit(-1);
    }

    FILE* outputFile = fopen("errorRec.bin", "wb");
    if (outputFile == NULL) {
        perror("Error creating output file");
        exit(-1);
    }

    fseek(inputFile, 0, SEEK_END);
    long fileSize = ftell(inputFile);
    fseek(inputFile, 0, SEEK_SET);

    int x[RS_MAX_POLY_DEGREE-EXTRA_POINTS];
    int y[RS_MAX_POLY_DEGREE-EXTRA_POINTS];
    int dataLen = sizeof(x)/sizeof(int);

    for(int i = 0; i < dataLen; i++)    x[i] = i;

    long filePosition = 0;
    while(filePosition < fileSize){
        printLoadingBar(filePosition, fileSize);
        memset(y, 0, sizeof(y));

        for(int i = 0; i < dataLen; i++){
            unsigned char read = fgetc(inputFile);

            if(read == EOF) break;

            y[i] = read; 
        }

        int xx[RS_MAX_POLY_DEGREE];
        int yy[RS_MAX_POLY_DEGREE + 1];
        addErrorCorrectionFields(x, y, dataLen, xx, yy);

        for(int j = dataLen; j < dataLen+EXTRA_POINTS+1; j++){
            unsigned char putData = yy[j] & 0xFF;
            int writeResult = fputc(putData, outputFile);
            if(writeResult == EOF || writeResult != writeResult){
                printf("The program is not writing properly\n");
                exit(-1);
            }
        }

        filePosition += dataLen;
        fseek(inputFile, filePosition, SEEK_SET);
    }
    printLoadingBar(fileSize, fileSize);

    if(filePosition >= fileSize){
        printf("\nFile completely error proofed!\n");
    }else{
        printf("\nFile wasn't completely processed!\n");
    }

    fclose(inputFile);
    fclose(outputFile);
}

void recuperateFile(const char* inputFilename, const char* recuperationFilename){
    FILE* inputFile = fopen(inputFilename, "rb");
    if (inputFile == NULL) {
        perror("Error opening input file");
        exit(-1);
    }

    FILE* recFile = fopen(recuperationFilename, "rb");
    if (recFile == NULL) {
        perror("Error opening the recuperation file");
        exit(-1);
    }

    FILE* outputFile = fopen("fixed.bin", "wb");
    if (outputFile == NULL) {
        perror("Error creating output file");
        exit(-1);
    }

    fseek(inputFile, 0, SEEK_END);
    long inputFilesize = ftell(inputFile);
    fseek(inputFile, 0, SEEK_SET);

    fseek(recFile, 0, SEEK_END);
    long recFilesize = ftell(recFile);
    fseek(recFile, 0, SEEK_SET);

    int x[RS_MAX_POLY_DEGREE];
    int y[RS_MAX_POLY_DEGREE+1];
    int dataLen = sizeof(x)/sizeof(int);

    for(int i = 0; i < dataLen; i++)    x[i] = i;

    long filePosition = 0;
    long correctionPosition = 0;
    while(filePosition < inputFilesize && correctionPosition < recFilesize){
        printLoadingBar(filePosition, inputFilesize);
        memset(y, 0, sizeof(y));

        for(int i = 0; i < dataLen-EXTRA_POINTS; i++){
            unsigned char read = fgetc(inputFile);

            if(read == EOF) break;

            y[i] = read; 
        }

        for(int i = dataLen-EXTRA_POINTS; i < dataLen+1; i++){
            unsigned char read = fgetc(recFile);

            if(read == EOF) break;

            y[i] = read; 
        }

        int success = verifyMessage(x, y, dataLen, RS_MAX_POLY_DEGREE-EXTRA_POINTS);
        if(success < 0){
            printf("\nError fixing the file at: 0x%08lX. Correction file position: 0x%08lX.\nData: ", filePosition, correctionPosition);
            for(int i = 0; i <= dataLen; i++){
                if(i==dataLen-EXTRA_POINTS) printf(" - ");
                printf("%02X", y[i]);
            }
            printf("\n");
        }

        for(int j = 0; j < RS_MAX_POLY_DEGREE-EXTRA_POINTS; j++){
            char putData = y[j] & 0xFF;
            fputc(putData, outputFile);
        }

        filePosition += dataLen-EXTRA_POINTS;
        fseek(inputFile, filePosition, SEEK_SET);
        correctionPosition += EXTRA_POINTS+1;
        fseek(inputFile, filePosition, SEEK_SET);
    }
    printLoadingBar(inputFilesize, inputFilesize);

    if(filePosition >= inputFilesize && correctionPosition >= recFilesize){
        printf("\nCorrection completed!\n");
    }else{
        printf("\nThe files were misaligned or an external error happened!\n");
        printf("Input: %ld/%ld, Correction: %ld/%ld\n", 
            filePosition, inputFilesize, correctionPosition, recFilesize);
    }

    fclose(inputFile);
    fclose(recFile);
    fclose(outputFile);
}

/***************************************************************************************************
 * MAIN
 **************************************************************************************************/
int main(){
    // testBench(10000);
    // testCase();

    createRecuperationFile("C:\\Users\\dabc\\repos\\CodingQuestions\\original.bin");
    recuperateFile("C:\\Users\\dabc\\repos\\CodingQuestions\\corrupted.bin",
        "C:\\Users\\dabc\\repos\\CodingQuestions\\ReedSalomon\\errorRec.bin");
}
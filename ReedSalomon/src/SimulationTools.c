/***************************************************************************************************
 * @file SimulationTools.c
 * @brief Tools to test the algorithm by generating random data or specifying your own.
 *
 * @version   1.0
 * @date      2024-07-23
 * @author    @dabecart
 *
 * @license
 * This project is licensed under the MIT License - see the LICENSE file for details.
 **************************************************************************************************/

#include "SimulationTools.h"

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

AlgorithmReturn runSimulation(int* x, int* y, int numPoints, int* errX, int* errY, int numErrors){
    int xx[numPoints + EXTRA_POINTS];
    int yy[numPoints + EXTRA_POINTS + 1];

    // Add to the message the error correction fields.
    addErrorCorrectionFields(x, y, numPoints, xx, yy);

    // Up to this point, the points are generated and "sent".
    int errory[numPoints + EXTRA_POINTS + 1];
    memcpy(errory, yy, sizeof(yy));
    
    // New errors are introduced!
    for(int i = 0; i < numErrors; i++){
        if(EEPROM_NOT_CORRUPTED && errX[i] >= numPoints){
            printf("If EEPROM is not corrupted, there cannot be errors and the EEPROM side.\n");
            exit(-1);
        }
        errory[errX[i]] = errY[i];
    }

    int ry[numPoints + EXTRA_POINTS + 1];
    memcpy(ry, errory, sizeof(errory));

    // Find the error.
    AlgorithmReturn success = verifyMessage(xx, ry, numPoints+EXTRA_POINTS, numPoints);
    if(success > 0){
        int sameAsSent = 1;
        for(int i = 0; (i < numPoints+EXTRA_POINTS) && sameAsSent; i++){
            sameAsSent &= yy[i] == ry[i];
        }
        if(!sameAsSent){
            if(numErrors >= EXTRA_POINTS)   success = FIXED_INCORRECTLY_EXCEEDS_NUMBER_OF_ERRORS;
            else                            success = FIXED_INCORRECTLY;
        }
    }else if(success == COULDNT_BE_FIXED && numErrors >= EXTRA_POINTS){
        success = EXCEEDS_NUMBER_OF_ERRORS;
    }

    if((PRINT_NON_FIXABLE_INPUTS && success == COULDNT_BE_FIXED) ||
        (PRINT_INCORRECTLY_FIXED_INPUTS && 
        (success == FIXED_INCORRECTLY || success == FIXED_INCORRECTLY_EXCEEDS_NUMBER_OF_ERRORS))){
        printf("NOT FIXED!\n");
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

AlgorithmReturn createSimulation(int numPoints, int minErrors, int maxErrors){
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

void testBench(int totalTests, int maxErrors){
    srand(time(0));
    printf("Number of tests:   %d\n", totalTests);
    printf("Points per sample: %d\n", RS_MAX_POLY_DEGREE-EXTRA_POINTS);
    printf("Number of errors:  rand[%d, %d]\n", 1, maxErrors);
    printf("#############  TEST BEGIN  ###############\n");
    int success = 0;
    int errorsExceedMaximum = 0;
    int fixedIncorrectly = 0;
    int fixedIncorrectlyExceedsNumberErrors = 0;

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
        AlgorithmReturn result = createSimulation(RS_MAX_POLY_DEGREE-EXTRA_POINTS, 0, maxErrors);
        
        if (clock_gettime(CLOCK_REALTIME, &t1) != 0) {
            perror("clock_gettime");
            return;
        }

        success += result > 0;
        fixedIncorrectly += (result == FIXED_INCORRECTLY);
        errorsExceedMaximum += (result == EXCEEDS_NUMBER_OF_ERRORS || result == FIXED_INCORRECTLY_EXCEEDS_NUMBER_OF_ERRORS);
        fixedIncorrectlyExceedsNumberErrors += (result == FIXED_INCORRECTLY_EXCEEDS_NUMBER_OF_ERRORS);

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

    printf("\n############# TEST RESULTS ###############\n");
    printf("Success rate: %d/%d. Fixed incorrectly: %d.\n"
           "Exceeding error limit: %d. Fixed incorrectly: %d.\n", 
           success, totalTests-errorsExceedMaximum, fixedIncorrectly, errorsExceedMaximum, fixedIncorrectlyExceedsNumberErrors);
    double bitRate = NUM_POINTS_SAMPLE*totalTests*1e9/averageElapsed;
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
    int y[] = { 0x9a, 
                0x48, 
                0x3e, 
                0x35, 
                0x27, 
                0xa8, 
                0x78, 
                0xe9, 
                0x64, 
                0x91};
    int numPoints = sizeof(y)/sizeof(int);

    int x[numPoints];
    for(int i = 0; i < numPoints; i++){
        x[i] = i;
    }

    int errX[] = {4,8};
    int errY[] = {0xe9, 0xf3};
    int numErrors = sizeof(errX)/sizeof(int);

    return runSimulation(x, y, numPoints, errX, errY, numErrors);
}
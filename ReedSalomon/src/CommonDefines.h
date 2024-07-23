/***************************************************************************************************
 * @file CommonDefines.h
 * @brief Auxiliary functions, tools and handlers to control the algorithm. 
 *
 * @version   1.0
 * @date      2024-07-23
 * @author    @dabecart
 *
 * @license
 * This project is licensed under the MIT License - see the LICENSE file for details.
 **************************************************************************************************/

#ifndef COMMON_DEFINES_h
#define COMMON_DEFINES_h

/***************************************************************************************************
 * ALGORITHM DEFINES
 **************************************************************************************************/

// Number of points per sample.
#define NUM_POINTS_SAMPLE       10
// The number of errors that will be fixed by the algorithm.
#define NUM_FIXABLE_ERRORS      2
// The modulus the algorithm will use. This has to be a PRIME!!!
#define MODULUS                 257

// If we take that the extra points are not corrupted, the computations are easier and faster to do.
// When the EEPROM is corrupted, we don't take into account the Hamming and CRC codes, as they are 
// redundant checks used to fix >2 bit length errors.
#define EEPROM_NOT_CORRUPTED    1

// Number of extra points to evaluate on the polynomial. 
// The number of errors that can be fixed are up to EXTRA_POINTS-1. Errors will be detected for up 
// to EXTRA_POINTS, but they will not be fixable. 
#define EXTRA_POINTS            NUM_FIXABLE_ERRORS+1

// The maximum degree of polynomials in this program.
#define RS_MAX_POLY_DEGREE      NUM_POINTS_SAMPLE+EXTRA_POINTS

/***************************************************************************************************
 * SIMULATION DEFINES
 **************************************************************************************************/

// Used for the input values. If inputs are bytes, this value will be 255. 
#define MAX_DATA_VALUE      255

// Length of the loading bar.
#define BAR_WIDTH           50

// Print all inputs that couldn't be fixed.
#define PRINT_NON_FIXABLE_INPUTS 0

// Print all inputs that were fixed incorrectly.
#define PRINT_INCORRECTLY_FIXED_INPUTS 0

/***************************************************************************************************
 * ALGORITHM RETURN TYPES
 **************************************************************************************************/

typedef enum{
    // When the algorithm fixes the message incorrectly and it had more errors than the maximum that
    // the algorithm can detect.
    FIXED_INCORRECTLY_EXCEEDS_NUMBER_OF_ERRORS = -4,

    // When the number of errors added to the message exceeds the maximum detectable by the 
    // algorithm.
    EXCEEDS_NUMBER_OF_ERRORS = -3,  

    // Message was fixed incorrectly (the algorithm thought it was OK but when contrasting it with 
    // the original message failed).
    FIXED_INCORRECTLY = -2,
    
    // Message couldn't be fixed.
    COULDNT_BE_FIXED = -1,

    UNDEFINED = 0,
    
    // Message is fine.
    WITHOUT_ERRORS = 1,

    // Message had errors, but they were fixed.
    FIXED_OK = 2,
} AlgorithmReturn;

/***************************************************************************************************
 * FUNCTIONS
 **************************************************************************************************/

// Simple function to print a loading bar. Hides the cursor until completed.
void printLoadingBar(long progress, long total);

#endif //COMMON_DEFINES_h
/***************************************************************************************************
 * @file SimulationTools.h
 * @brief Tools to test the algorithm by generating random data or specifying your own.
 *
 * @version   1.0
 * @date      2024-07-23
 * @author    @dabecart
 *
 * @license
 * This project is licensed under the MIT License - see the LICENSE file for details.
 **************************************************************************************************/

#ifndef SIMULATION_TOOLS_h
#define SIMULATION_TOOLS_h

#include "CommonDefines.h"
#include "ReedSalomon.h"
#include <time.h>

/***************************************************************************************************
 * FUNCTIONS
 **************************************************************************************************/

// Create a bunch of random tests and specify the maximum number of errors introduced that the 
// algorithm will try to fix.
void testBench(int totalTests, int maxErrors);

// Runs a single case hardcoded in this function.
int testCase();

#endif //SIMULATION_TOOLS_h
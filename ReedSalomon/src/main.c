/***************************************************************************************************
 * @file main.c
 * @brief Launch point for the program.
 *
 * @version   1.0
 * @date      2024-07-23
 * @author    @dabecart
 *
 * @license
 * This project is licensed under the MIT License - see the LICENSE file for details.
 **************************************************************************************************/

#include "SimulationTools.h"
#include "FileTools.h"

/***************************************************************************************************
 * MAIN
 **************************************************************************************************/
int main(){
    testBench(10000, EXTRA_POINTS);
    
    // testCase();

    // createRecuperationFile("C:\\Users\\dabc\\repos\\CodingQuestions\\ReedSalomon\\original.bin");
 
    // recuperateFile("C:\\Users\\dabc\\repos\\CodingQuestions\\ReedSalomon\\corrupted.bin",
    //     "C:\\Users\\dabc\\repos\\CodingQuestions\\ReedSalomon\\errorRec.bin");
}
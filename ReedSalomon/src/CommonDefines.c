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

#include "CommonDefines.h"
#include <stdio.h>

void printLoadingBar(long progress, long total) {
    int barLen = (progress * BAR_WIDTH) / total;
    // First expression hides the cursor.
    printf("\e[?25lProgress: [");
    for (int i = 0; i < BAR_WIDTH; ++i) {
        if (i < barLen) {
            printf("#");
        } else {
            printf(" ");
        }
    }
    printf("] %ld%%: %ld/%ld\r", (progress * 100) / total, progress, total);
    // If ended, show the cursor again.
    if(progress >= total)   printf("\e[?25h");
    fflush(stdout);
}

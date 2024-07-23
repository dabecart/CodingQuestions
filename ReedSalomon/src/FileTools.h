/***************************************************************************************************
 * @file FileTools.h
 * @brief Tools to run the ReedSalomon algorithm on files. 
 *
 * @version   1.0
 * @date      2024-07-23
 * @author    @dabecart
 *
 * @license
 * This project is licensed under the MIT License - see the LICENSE file for details.
 **************************************************************************************************/

#ifndef FILE_TOOLS_h
#define FILE_TOOLS_h

#include "CommonDefines.h"
#include "ReedSalomon.h"

/***************************************************************************************************
 * FUNCTIONS
 **************************************************************************************************/

// Creates the file that will contain the extra data to correct [filename] in case it gets 
// corrupted.
void createRecuperationFile(const char* filename);

// Tries to recuperate [inputFilename] with the [recuperationFilename] file. 
void recuperateFile(const char* inputFilename, const char* recuperationFilename);

#endif
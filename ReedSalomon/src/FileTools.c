/***************************************************************************************************
 * @file FileTools.c
 * @brief Tools to run the ReedSalomon algorithm on files. 
 *
 * @version   1.0
 * @date      2024-07-23
 * @author    @dabecart
 *
 * @license
 * This project is licensed under the MIT License - see the LICENSE file for details.
 **************************************************************************************************/

#include "FileTools.h"

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
    long blocksCorrected = 0;
    long totalBlocks = 0;
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
        }else{
            blocksCorrected++;
        }
        totalBlocks++;

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
        printf("\nCorrection completed! %ld of %ld blocks OK!\n", blocksCorrected, totalBlocks);
    }else{
        printf("\nThe files were misaligned or an external error happened!\n");
        printf("Input: %ld/%ld, Correction: %ld/%ld\n", 
            filePosition, inputFilesize, correctionPosition, recFilesize);
    }

    fclose(inputFile);
    fclose(recFile);
    fclose(outputFile);
}
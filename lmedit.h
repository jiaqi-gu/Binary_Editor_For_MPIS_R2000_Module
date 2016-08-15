/*
**   lmedit.h
**
**   Edit header and store info in exec struct.
**
**   Author: Jiaqi Gu (jxg4836@rit.edu)
*/

#ifndef LMEDIT_H
#define LMEDIT_H

#include "exec.h"
#include "header.h"
#include "tables.h"
#include "editor.h"

/// function for printing data
exec_t header;

/// file to be read
FILE* pFile;

/// store data from text and data sections
uint8_t* textdata;

/// tables to store data
relent_t* relTable;
refent_t* refTable;
syment_t* symTable;
char* str;

/// function to print information
///
/// @param char* file
/// @returns 0 if no error
int printInfo(char* file);

/// a function to analyze a R2K file
///
/// @param char* file
/// @returns 0 if no error
int Analysis(char* file);

/// main function for the program
///
/// @param void
/// @returns 0 if no error
int printsection();

#endif

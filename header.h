/*
**   header.h
**
**   Read header and store info in exec struct.
**
**   Also, provide a function to print Header information
**
**   Author: Jiaqi Gu (jxg4836@rit.edu)
*/

#ifndef HEADER_H
#define HEADER_H

/// hex value to help split year, month and date in a binary data
#define    YEAR    0xfe00
#define    MONTH    0x1e0
#define    DATE    0x1f

#include "exec.h"
#include "lmedit.h"

/// print data info in header
///
/// @param void
/// @returns 0 if no error
int printData();

/// initialize header struct
///
/// @param void
/// @returns 0 if no error
int iniheader();

/// a function to analysis Header
///
/// @param char* file
/// @returns 0 if no error
int HeaderAnalysis(char* file);

#endif

/*
**   tables.h
**
**   Read tables and store info in relent, refent, and syment respectively.
**
**   Also, provide a function to print tables information
**
**   Author: Jiaqi Gu (jxg4836@rit.edu)
*/

#ifndef TABLES_H
#define TABLES_H

#include "exec.h"
#include "lmedit.h"

/// print data info in header
///
/// @param void
/// @returns 0 if no error
int printrel(relent_t);

/// print data info in header
///
/// @param void
/// @returns 0 if no error
int printref(refent_t);

/// print data info in header
///
/// @param void
/// @returns 0 if no error
int printsym(syment_t);

/// print data info in header
///
/// @param void
/// @returns 0 if no error
int printTables();

/// free dynamical allocation
///
/// @param void
/// @returns 0 if no error
int freeTables();

/// a function to analysis Header
///
/// @param void
/// @returns 0 if no error
int TableAnalysis();

#endif

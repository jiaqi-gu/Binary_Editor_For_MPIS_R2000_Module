/*
** Use for editing
**
** editor.h
**
** Author: Jiaqi Gu (jxg4836@rit.edu)
*/
#ifndef EDITOR_H
#define EDITOR_H

#include "exec.h"
#include "lmedit.h"
#include "tables.h"

///for binary convert use
#define eight 0x00000003

///for checking if A/N/T/V empty
#define EMPTY 0xFFFFFFFF

///declare
char buf[128];
char sect[32];
char sect1[32];

int section; ///current section
int count;   ///total count
int repeat;  ///for repeating history use

/// A[,N][:T][=V]
uint32_t A, A1; //A is index, and A1 is address
uint32_t N;
uint32_t T;  /// 1 - bits, 2 -half, 3- full
uint32_t V;
char A0[30];
char N0[30];
char T0[30];
char V0[30];

/// for write use
uint32_t writenaddress; /// this is the number of bytes to be wrote
uint32_t writevalue; ///this is the value to be assign
int writeindex; /// this is the index from the beginning of the file
int writenumber; /// this is the number of bytes to be wrote
int ifwrite; ///1 if "=V" is used and value has not been assigned yet, otherwise 0
int writetype; ///1 - bits, 2 - half word, 3 - full
char writebuf[100];

// struct for history functionality
typedef struct his{
    int index;
    char content[100];
}his_t;

// array for storing histories
his_t history[10];

/// function for editing mode
///
/// @param void
/// @returns 0 if no error
int editor();

#endif

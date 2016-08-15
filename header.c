/*
**   header.c
**
**   Read header and store info in exec struct.
**
**   Also, provide a function to print Header information
**
**   Author: Jiaqi Gu (jxg4836@rit.edu)
*/

#define _BSD_SOURCE
#include "endian.h"

#include <stdio.h>   //standard input/output
#include <stdlib.h>  //standard c library
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <stdint.h>

#include "header.h"

/// print data info in header
///
/// @param void
/// @returns 0 if no error
int printData()
{
    if(header.data[0]!=0){printf("Section text is %d bytes long\n",header.data[0]);}
    if(header.data[1]!=0){printf("Section rdata is %d bytes long\n",header.data[1]);}
    if(header.data[2]!=0){printf("Section data is %d bytes long\n",header.data[2]);}
    if(header.data[3]!=0){printf("Section sdata is %d bytes long\n",header.data[3]);}
    if(header.data[4]!=0){printf("Section sbss is %d bytes long\n",header.data[4]);}
    if(header.data[5]!=0){printf("Section bss is %d bytes long\n",header.data[5]);}
    if(header.data[6]!=0){printf("Section reltab is %d entries long\n",header.data[6]);}
    if(header.data[7]!=0){printf("Section reftab is %d entries long\n",header.data[7]);}
    if(header.data[8]!=0){printf("Section symtab is %d entries long\n",header.data[8]);}
    if(header.data[9]!=0){printf("Section strings is %d bytes long\n",header.data[9]);}
    return 0;
}

/// initialize header struct
///
/// @param void
/// @returns 0 if no error
int iniheader()
{
    header.magic = 0;
    header.version = 0;
    header.flags = 0;
    header.entry = 0;
    for(int i=0;i<10;++i)
    {
        header.data[i]=0;
    }
    return 0;
}

/// a function to analysis Header
///
/// @param char* file
/// @returns 0 if no error
int HeaderAnalysis(char* file){
	    
	fread(&(header.magic),2,1,pFile);
	header.magic = be16toh(header.magic);
	if(header.magic!=HDR_MAGIC){
	    fprintf(stderr, "error: %s is not an R2K object module (magic number 0x%08x)\n", file, header.magic);
	    return 1;
	}
	
	/// get data for version flags and entry
	fread(&(header.version),2,1,pFile);
	header.version = be16toh(header.version);
	
	fread(&(header.flags),4,1,pFile);
	header.flags = be32toh(header.flags);
	
	fread(&(header.entry),4,1,pFile);
	header.entry = be32toh(header.entry);
	
	for(int i=0;i<10;++i)
	{
	    fread(&(header.data[i]),4,1,pFile);
	    header.data[i]=be32toh(header.data[i]);
	}
	
	return 0;
}

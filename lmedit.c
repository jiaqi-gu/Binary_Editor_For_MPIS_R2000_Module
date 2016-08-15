/*
**   lmedit.c
**
**   Edit R2000K Architecture Object
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

#include "lmedit.h"

/// function to print information
///
/// @param char* file
/// @returns 0 if no error
int printInfo(char* file){    
    /// print file basic info
    if(header.entry == 0){//object module
        printf("File %s is an R2K object module\n",file);
    }else{
        printf("File %s is an R2K load module  (entry point 0x%08x)\n",file,header.entry);
    }
    
    /// print module version
    printf("Module version:  2%03d/%02d/%02d\n",(header.version & YEAR)>>9,(header.version & MONTH)>>5,(header.version & DATE));
    /// print data sections
    printData();
    /// print tables
    //printTables();
    return 0;
}

/// a function to analyze a R2K file
///
/// @param char* file
/// @returns 0 if no error
int Analysis(char* file)
{
    pFile = fopen(file, "r+");
    
    if (!pFile) 
    { 
		perror(file);
		return 1;
	}else{
	    if(HeaderAnalysis(file)==1){
	        fclose(pFile);
	        return 1;
	    }
	    TableAnalysis(pFile);
	}
	
	printInfo(file);
	///freeTables();
	///fclose(pFile);
	
	return 0;
}

/// main function for the program
///
/// @param void
/// @returns 0 if no error
int printsection(){
    if(section==0){//text
        printf("text[%d] >",count);
    }else if(section==1){/// rdata
        printf("rdata[%d] >",count);
    }else if(section==2){/// data
        printf("data[%d] >",count);
    }else if(section==3){/// sdata
        printf("sdata[%d] >",count);
    }else if(section==4){/// sbss
        printf("sbss[%d] >",count);
    }else if(section==5){/// bss
        printf("bss[%d] >",count);
    }else if(section==6){/// reltab
        printf("reltab[%d] >",count);
    }else if(section==7){/// reftab
        printf("reftab[%d] >",count);
    }else if(section==8){/// symtab
        printf("symtab[%d] >",count);
    }else if(section==9){/// string
        printf("string[%d] >",count);
    }
    return 0;
}

/// main function for the program
///
/// @param argc  2
/// @param argv  program name, file name
/// @returns errorCode  the error Code; 0 if no error
int main(int argc, char *argv[])
{
    /// check arguments
    if(argc<=1)
    { /// if missing arguments, print error and exit
        fprintf(stderr, "usage: lmedit file\n");
        exit(EXIT_FAILURE);
    }
    
    /// open each file and analyze if available
    for(int n=1;n<argc;++n)
    {
        iniheader();
        ///use analysis function
	    if(Analysis(argv[n]) == 1){
	        return 1;
	    }
    }
    
    section = 0;
    count = 1;
    
    /// loop until user enter "quit"
    while(1)
    {
        printsection();
    
        if(editor()==9){
            return 0;
        }
        
        ++count;
    }
    
    return EXIT_SUCCESS;
}

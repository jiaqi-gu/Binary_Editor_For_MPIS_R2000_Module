/*
**   tables.c
**
**   Read tables and store info in relent, refent, and syment respectively.
**
**   Also, provide a function to print tables information
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

#include "tables.h"

//using for splitting into smaller bit units
uint16_t u16;
uint32_t u32;
#define    front8    0xff00
#define    least8    0xff

/// print data info in header
///
/// @param void
/// @returns 0 if no error
int printrel(relent_t rel){
    if((int)rel.section == 1){
        printf("   0x%08x (text) type 0x%04x\n",rel.addr,rel.type);
        return 0;
    }
    if((int)rel.section == 2){
        printf("   0x%08x (rdata) type 0x%04x\n",rel.addr,rel.type);
        return 0;
    }
    if((int)rel.section == 3){
        printf("   0x%08x (data) type 0x%04x\n",rel.addr,rel.type);
        return 0;
    }
    if((int)rel.section == 4){
        printf("   0x%08x (sdata) type 0x%04x\n",rel.addr,rel.type);
        return 0;
    }
    if((int)rel.section == 5){
        printf("   0x%08x (sbss) type 0x%04x\n",rel.addr,rel.type);
        return 0;
    }
    if((int)rel.section == 6){
        printf("   0x%08x (bss) type 0x%04x\n",rel.addr,rel.type);
        return 0;
    }
    printf("   0x%08x 0x%04x type 0x%04x\n",rel.addr,rel.section,rel.type);
    return 0;
}

/// print data info in header
///
/// @param void
/// @returns 0 if no error
int printref(refent_t ref){
    printf("   0x%08x type 0x%04x symbol ",ref.addr,ref.type);
    int index = ref.sym;
    while(1){
        printf("%c",str[index]);
        index++;
        if(str[index]==0){
            printf("\n");
            return 0;
        }
    }
    return 0;
}

/// print data info in header
///
/// @param void
/// @returns 0 if no error
int printsym(syment_t sym){
    printf("   value 0x%08x flags 0x%08x symbol ",sym.value,sym.flags);
    int index = sym.sym;
    while(1){
        printf("%c",str[index]);
        index++;
        if(str[index]==0){
            printf("\n");
            return 0;
        }
    }
    return 0;
}

/// print data info in header
///
/// @param void
/// @returns 0 if no error
int printTables()
{
    if(header.data[6] != 0)
        {/// Relocation table
            printf("Relocation information:\n");
            for(int i=0;i<(int)header.data[6];++i){
                printrel(relTable[i]);
            }
        }

        if(header.data[7] != 0)
        {/// Relocation table
            printf("Reference information:\n");
            for(int i=0;i<(int)header.data[6];++i){
                printref(refTable[i]);
            }
        }

        if(header.data[8] != 0)
        {/// Relocation table
            printf("Symbol table:\n");
            for(int i=0;i<(int)header.data[8];++i){
                printsym(symTable[i]);
            }
        }
    return 0;
}

/// free dynamical allocation
///
/// @param void
/// @returns 0 if no error
int freeTables()
{
    if(textdata){free(textdata);textdata=NULL;}
    if(relTable){free(relTable);relTable=NULL;}
    if(refTable){free(refTable);refTable=NULL;}
    if(symTable){free(symTable);symTable=NULL;}
    if(str){free(str);str=NULL;}
    return 0;
}

/// a function to analysis Header
///
/// @param void
/// @returns 0 if no error
int TableAnalysis(){
    /// pass through text and data contents
    ///fseek(pFile, (header.data[0]+header.data[1]+header.data[2]+header.data[3]+header.data[4]+header.data[5]) ,SEEK_CUR);
    
    /// read data
    int size = (int) (header.data[0] + header.data[1] +header.data[2] +header.data[3] +header.data[4] + header.data[5]);
    textdata = malloc(size * sizeof(uint8_t));
    fread(textdata,size,1,pFile);
    
    /// assign data to dynamic memory
    if(header.data[6] != 0)
    {/// Relocation table
        relTable = malloc(header.data[6] * sizeof(relent_t));
        fread(relTable,header.data[6] * sizeof(relent_t),1,pFile);

        ///convert
        for(int i=0;i<(int)header.data[6];++i){
            relTable[i].addr = be32toh(relTable[i].addr);
            u16 = be16toh(relTable[i].section);
            relTable[i].section = (u16 & front8)>>8;
        }
    }

    if(header.data[7] != 0)
    {/// Reference table
        refTable = malloc(header.data[7] * sizeof(refent_t));
        fread(refTable,header.data[7] * sizeof(refent_t),1,pFile);

        //convert
        for(int i=0;i<(int)header.data[7];++i){
            refTable[i].addr = be32toh(refTable[i].addr);
            refTable[i].sym = be32toh(refTable[i].sym);
            u16 = be16toh(refTable[i].section);
            refTable[i].section = (u16 & front8)>>8;
        }
    }

    if(header.data[8] != 0)
    {/// Symbol table
        symTable = malloc(header.data[8] * sizeof(syment_t));
        fread(symTable,header.data[8] * sizeof(syment_t),1,pFile);

        //convert
        for(int i=0;i<(int)header.data[8];++i){
            symTable[i].flags = be32toh(symTable[i].flags);
            symTable[i].value = be32toh(symTable[i].value);
            symTable[i].sym = be32toh(symTable[i].sym);
        }
    }
        
    if(header.data[9] != 0)
    {/// String
        str = malloc(header.data[9] * sizeof(char));
        fread(str,header.data[9] * sizeof(char),1,pFile);
    }

    return 0;
}

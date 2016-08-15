/*
**   editor.c
**
**   Editing mode
**
**   Author: Jiaqi Gu (jxg4836@rit.edu)
*/

#include <stdio.h>
#include <sys/select.h>
#include <string.h>
#include <stdint.h>

#include "editor.h"

/// helper function for getting input
///
/// @param void
/// @returns 0 if no error
static int getinput(char* buf, int length, FILE * fp)
{
    if(fgets(buf,length,fp) == NULL)
    {
        if ( feof( fp ) ) {
            return -1;
        }
        
        if ( ferror( fp ) ) {
            perror( "error getting input with fgets" );
        }
    }
    return 0;
}

///================================history functions below=============================================================

/// function for putting a new history in history array
///
/// @param void
/// @returns 0 if no error
static int newhistory()
{
    //check if history array is full of his
    if((history[9].index) ){  //if entries is full
        for(int i=0;i<9;i++){
            history[i].index = history[i+1].index;
            strcpy(history[i].content,history[i+1].content);
        }
        history[9].index = count;
        strcpy(history[9].content,buf);
        return 0;
    }
    
    //if not full, find a empty entry and put info in it
    for(int i=0;i<10;i++){
        if(! (history[i].index) ){
            history[i].index = count;
            strcpy(history[i].content,buf);
            return 0;
        }
    }
    return 0;
}

/// function for printing all histories
///
/// @param void
/// @returns 0 if no error
static int printhistory(){
    for(int i=0;i<10;i++){
        if((history[i].index) ){
            printf("%d  %s",history[i].index,history[i].content);
        }
    }
    /// printf("\n");
    return 0;
}

/// function for repeating a particular history
///
/// @param void
/// @returns 0 if no error
static int repeathistory(int in){
    /// if no long in history (in < history[0].index)
    if(in < history[0].index){
        fprintf( stderr, "error:  command %d is no longer in the command history\n", in);
        return -1;
    }
    /// if no long in history (in > history[8].index)
    int max=0;
    for(int i=0;i<10;i++){
        if((history[i].index)){
            max = i;
        }
    }
    ///printf("max:%d\n",max);
    if(in > history[max].index){
        fprintf( stderr, "error:  command %d has not yet been entered\n", in);
        return -1;
    }
    /// otherwise, repeat command
    for(int i=0;i<10;i++){
        if(history[i].index == in){
            strcpy(buf,history[i].content);
        }
    }
    /// print
    ++count;
    printsection();
    printf("%s",buf);
    return 0;
}

///================================write functions==================================================

/// helper function for writing in words
///  
/// @param 
/// @returns 0 if no error
static int writehelperF()
{   
    ///printf("   0x%08x is now 0x%08x\n",writenaddress,writevalue);
    fseek(pFile, writeindex ,SEEK_SET);
    fwrite(&writevalue,1,1,pFile);
    freeTables();
    fseek(pFile,sizeof(exec_t) ,SEEK_SET);
    TableAnalysis();
    return 0;
}

/// function for writing tables section
///  
/// @param 
/// @returns 0 if no error
int writeF()
{
    if(ifwrite==0){return 0;}
    for(int i = 0;i<writenumber;i++){
        writehelperF();
        A+=4;
    }
    ifwrite = 0; /// make no writable after writing
    return 0;
}

/// helper function for writing in words
///  
/// @param 
/// @returns 0 if no error
static int writehelperB()
{   
    ///printf("   0x%02x is now 0x%02x\n",writenaddress,writevalue);
    fseek(pFile, writeindex ,SEEK_SET);
    fwrite(&writevalue,1,1,pFile);
    freeTables();
    fseek(pFile,sizeof(exec_t) ,SEEK_SET);
    TableAnalysis();
    return 0;
}

/// function for writing tables section
///  
/// @param 
/// @returns 0 if no error
int writeB()
{
    if(ifwrite==0){return 0;}
    for(int i = 0;i<writenumber;i++){
        writehelperB();
        A+=1;
    }
    ifwrite = 0; /// make no writable after writing
    return 0;
}

///================================helper functions for analyser below==================================================

/// function for printing tables section
///  
/// @param int type( 6(rel),7(ref),8(sym),9(str) ), int number, int index
/// @returns 0 if no error
static int printTAB(int type, int number, int index)
{
    if(type==6){
        for(int i=0;i<number;i++){
            printrel(relTable[index]);
            index++;
        }
    }
    if(type==7){
        for(int i=0;i<number;i++){
            printref(refTable[i]);
            index++;
        }
    }
    if(type==8){
        for(int i=0;i<number;i++){
            printsym(symTable[i]);
            index++;
        }
    }
    return 0;
}

/// function for printing text and data sections
///  
/// @param int type( 1 (b), 2(h), 3(w) ), int number, int index
/// @returns 0 if no error
static int printtextdata(int type, int number, int index)
{
    if(type==1){ //byte
        for(int i=0;i<number;i++){
            printf("   0x%08x = 0x%02x\n",A1,textdata[index]);
            index+=1;
            A1+=2;
        }
    }
    if(type==2){ //half word - 2bytes
        for(int i=0;i<number;i++){
            printf("   0x%08x = 0x%02x",A1,textdata[index]);
            printf("%02x\n",textdata[index+1]);
            index+=2;
            A1+=2;
        } 
    }
    if(type==3){ //word - 4bytes
        for(int i=0;i<number;i++){
            printf("   0x%08x = 0x%02x",A1,textdata[index]);
            printf("%02x",textdata[index+1]);
            printf("%02x",textdata[index+2]);
            printf("%02x\n",textdata[index+3]);
            index+=4;
            A1+=4;
        }
    }
    return 0;
}

///================================analyser functions below=============================================================

/// function for analyzing object module
///  A[,N][:T][=V]                        T=1 (b), 2(h), 3(w)
/// @param void
/// @returns 0 if no error
static int object()
{ 
    /// a=-1
    if( (A==EMPTY) &&(N==EMPTY) && (T==EMPTY) && (V==EMPTY) ){
        if(header.entry!=0){
            fprintf( stderr, "error:  '0xffffffff' is not a valid address\n");
            return 1;
        }
        fprintf( stderr, "error:  '-1' is not a valid address\n");
        return 1;
    }
    
    
    /// 0 - Print a 32-bit word starting at location 0     (A)
    if( (A!=EMPTY) &&(N==EMPTY) && (T==EMPTY) && (V==EMPTY) ){
        int a = (int) A;
        
        ///===========table section below=============
        if( (section==6) || (section==7) || (section==8) ){
            if(  (a<0)  ||  (a>= ( (int)header.data[section])) ){
                sscanf(A0,"%s",sect);
                fprintf( stderr, "error:  '%s' is not a valid address\n", sect);
                return 1;
            }
            printTAB((int)section,1,(int)A); /// type, number, index
            return 0;
        }
        
        ///===========text and data sections below====
        /// check if index is valid
        if(  (a<0)  ||  (a>= ( (int)header.data[section]) -3 ) ){
            sscanf(A0,"%s",sect);
            fprintf( stderr, "error:  '%s' is not a valid address\n", sect);
            return 1;
        }
        
        if( (section==0) || (section==1) || (section==2) ||(section==3) ||(section==4) ||(section==5)||(section==9)){
            ///printf("section:%d\n",section);
            int index = (int) A;
            for(int i=0;i<section;i++){
                index+=(int)header.data[i];
            }
            printtextdata(3, 1, index);
        }
        
        return 0;
    }
    
    
    /// 0:h - Print a 16-bit halfword starting at location 0   (A:T)
    if( (A!=EMPTY) &&(N==EMPTY) && (T!=EMPTY) && (V==EMPTY) ){
        ///===========table section below=============
        if( (section==6) || (section==7) || (section==8) ){
            fprintf( stderr, "error:  ':%s' is not valid in table sections\n", T0);
            return 1;
        }
        
        ///===========text and data sections below====
        int a = (int) A;
        
        /// check if index is valid
        if(  (a<0)  ||  (a>=(int)header.data[section])  ){
            sscanf(A0,"%s",sect);
            fprintf( stderr, "error:  '%s' is not a valid address\n", sect);
            return 1;
        }
        
        if( (section==0) || (section==1) || (section==2) ||(section==3) ||(section==4) ||(section==5)||(section==9)){
            ///printf("section:%d\n",section);
            int index = (int) A;
            for(int i=0;i<section;i++){
                index+=(int)header.data[i];
            }
            ///printf("T:%d\n",T);
            printtextdata((int)T, 1, index);
        }
        
        return 0;
    }
    
    
    /// 20,10:b - Print 10 bytes starting at location 20  (A,N:T)
    if( (A!=EMPTY) &&(N!=EMPTY) && (T!=EMPTY) && (V==EMPTY) ){
        ///===========table section below=============
        if( (section==6) || (section==7) || (section==8) ){
            fprintf( stderr, "error:  ':%s' is not valid in table sections\n", T0);
            return 1;
        }
        
        ///===========text and data sections below====
        int a = (int) A;
        int n = (int) N;
        int m = 1;
        
        if((int)T == 2){
            m=2;
        }
        
        if((int)T == 3){
            m=4;
        }
        
        /// check if count is valid
        if(  (a<0)  ||  ((a+n*m)>(int)header.data[section])  ){
            fprintf( stderr, "error:  '%d' is not a valid count\n", N);
            return 1;
        }
        
        if( (section==0) || (section==1) || (section==2) ||(section==3) ||(section==4) ||(section==5)||(section==9)){
            ///printf("section:%d\n",section);
            int index = (int) A;
            for(int i=0;i<section;i++){
                index+=(int)header.data[i];
            }
            ///printf("T:%d\n",T);
            printtextdata((int)T, (int)N, index);
        }
        
        return 0;
    }
    
    
    /// 40,4 - Print 4 32-bit words starting at location 40  (A,N)
    if( (A!=EMPTY) &&(N!=EMPTY) && (T==EMPTY) && (V==EMPTY) ){
        int a = (int) A;
        int n = (int) N;
        
        ///===========table section below=============
        if( (section==6) || (section==7) || (section==8) ){
            if(  (a<0)  ||  (a> ( (int)header.data[section])) ){
                sscanf(A0,"%s",sect);
                fprintf( stderr, "error:  '%s' is not a valid address\n", sect);
                return 1;
            }
            if(  (a<0)  ||  (a+n> ( (int)header.data[section])) ){
                fprintf( stderr, "error:  '%d' is not a valid count\n", N);
                return 1;
            }
            printTAB((int)section,n,a); /// type, number, index
            return 0;
        }
        
        ///===========text and data sections below====
        int m = 0;
        
        if((int)T == 2){
            m=1;
        }
        
        if((int)T == 3){
            m=3;
        }
        
        /// check if index is valid
        if(  (a<0)  ||  (a>=(int)header.data[section] - m)  ){
            sscanf(A0,"%s",sect);
            fprintf( stderr, "error:  '%s' is not a valid address\n", sect);
            return 1;
        }
        
        /// check if count is valid
        if(  (a<0)  ||  ((a+n)>=(int)header.data[section])  ){
            fprintf( stderr, "error:  '%d' is not a valid count\n", N);
            return 1;
        }
        
        if( (section==0) || (section==1) || (section==2) ||(section==3) ||(section==4) ||(section==5) ||(section==9)){
            ///printf("section:%d\n",section);
            int index = (int) A;
            for(int i=0;i<section;i++){
                index+=(int)header.data[i];
            }
            ///printf("T:%d\n",T);
            printtextdata(3, (int)N, index);
        }
        
        return 0;
    }
    
    
    /// 0,4:b=5 Assign the value 5 to each of 4 bytes beginning at location 0 (A,N:T=V)
    if( (A!=EMPTY) &&(N!=EMPTY) && (T!=EMPTY) && (V!=EMPTY) ){
        ///===========table section below=============
        if( (section==6) || (section==7) || (section==8)){
            fprintf( stderr, "error:  ':%s' is not valid in table sections\n", T0);
            return 1;
        }
        
        ///check address and count
        int a = (int) A;
        int n = (int) N;
        int m = 0;
        if((int)T == 2){m=1;}
        if((int)T == 3){m=3;}
        /// check if index is valid
        if(  (a<0)  ||  (a>=(int)header.data[section] - m)  ){
            sscanf(A0,"%s",sect);
            fprintf( stderr, "error:  '%s' is not a valid address\n", sect);
            return 1;
        }
    
        if(  (a<0)  ||  ((a+n)>=(int)header.data[section])  ){
            fprintf( stderr, "error:  '%d' is not a valid count\n", N);
            return 1;
        }
    
        ///===========write below====
        writevalue = V;
        writenaddress = A;
        writenumber = (int)N;
        ifwrite = 1;
        writeindex = sizeof(exec_t) + (int)A;
        if(T==1){///bits
            writetype = 1;
            uint32_t tempadd = writenaddress;
            for(int i=0;i<writenumber;i++){
                printf("   0x%02x is now 0x%02x\n",tempadd,writevalue);
                tempadd+=1;
            }
        }
        if(T==2){///half
            writetype = 2;
            writeindex+=1;
            uint32_t tempadd = writenaddress;
            for(int i=0;i<writenumber;i++){
                printf("   0x%04x is now 0x%04x\n",tempadd,writevalue);
                tempadd+=4;
            }
        }
        if(T==3){///full
            writetype = 3;
            writeindex+=3;
            uint32_t tempadd = writenaddress;
            for(int i=0;i<writenumber;i++){
                printf("   0x%08x is now 0x%08x\n",tempadd,writevalue);
                tempadd+=4;
            }
        }
        for(int i=0;i<section;i++){
            writeindex+=(int)header.data[i];
        }
        return 0;
    }
    
    
    /// 20=42 Assign the value 42 to the 32-bit word at location 20 (A=V)
    if( (A!=EMPTY) &&(N==EMPTY) && (T==EMPTY) && (V!=EMPTY) ){
        ///===========table section below=============
        if( (section==6) || (section==7) || (section==8) ){
            fprintf( stderr, "error:  '=%s' is not valid in table sections\n", V0);
            return 1;
        }
        
        ///===========text and data sections below====
        writevalue = V;
        writenaddress = A;
        writeindex = sizeof(exec_t) + (int)A + 3; ///since it's big endian, we need to write at the last 8 bits of 32 bits
        for(int i=0;i<section;i++){
                writeindex+=(int)header.data[i];
        }
        writenumber = 1;
        ifwrite = 1;
        writetype = 3; ///full words
        
        printf("   0x%08x is now 0x%08x\n",writenaddress,writevalue);
        
        return 0;
    }
    
    /// 20,10=b - write 10 bytes starting at location 20  (A,N=T)
    if( (A!=EMPTY) &&(N!=EMPTY) && (T==EMPTY) && (V!=EMPTY) ){
        ///===========table section below=============
        if( (section==6) || (section==7) || (section==8) ){
            fprintf( stderr, "error:  '=%s' is not valid in table sections\n", V0);
            return 1;
        }

        ///check address and count
        int a = (int) A;
        int n = (int) N;
        int m = 0;
        if((int)T == 2){m=1;}
        if((int)T == 3){m=3;}
        /// check if index is valid
        if(  (a<0)  ||  (a>=(int)header.data[section] - m)  ){
            sscanf(A0,"%s",sect);
            fprintf( stderr, "error:  '%s' is not a valid address\n", sect);
            return 1;
        }
    
        if(  (a<0)  ||  ((a+n)>=(int)header.data[section])  ){
            fprintf( stderr, "error:  '%d' is not a valid count\n", N);
            return 1;
        }
    
        ///===========write below====
        writevalue = V;
        writenaddress = A;
        writeindex = sizeof(exec_t) + (int)A+3; ///since it's big endian, we need to write at the last 8 bits of 32 bits
        for(int i=0;i<section;i++){
            writeindex+=(int)header.data[i];
        }
        writenumber = (int)N;
        ifwrite = 1;
        writetype = 3; ///full
        
        uint32_t tempadd = writenaddress;
        for(int i=0;i<writenumber;i++){
            printf("   0x%08x is now 0x%08x\n",tempadd,writevalue);
            tempadd+=4;
        }
        
    }
    
    return 0;
}


/// function for analyzing load module
///
/// @param void
/// @returns 0 if no error
static int load()
{
    /// a=-1
    if( (A==EMPTY) &&(N==EMPTY) && (T==EMPTY) && (V==EMPTY) ){
        if(header.entry!=0){
            fprintf( stderr, "error:  '0xffffffff' is not a valid address\n");
            return 1;
        }
        fprintf( stderr, "error:  '-1' is not a valid address\n");
        return 1;
    }
    
    if( A!=EMPTY){
        if(section==0){///text
            A-=TEXT_BEGIN;
        }
        if(section==1){///rdata
            A-=DATA_BEGIN;
        }
        if(section==2){///data
            A-=DATA_BEGIN;
            A-= header.data[1]; /// take out the space that data occupied
        }
        if(section==3){///sdata
            A-=DATA_BEGIN;
            A-=header.data[1];
            A-=header.data[2];
        }
        if(section==9){///str
            ///do nothing
        }
    }
    
    ///treat as an object except address
    object();
    
    return 0;
}

///================================editor function below=============================================================



/// function for editing mode
///
/// @param void
/// @returns 0 if no error
int editor(){
    /// get input
    getinput(buf,128,stdin);
    /// printf("Input: %s",buf);
    
    newhistory();
    
    if( strcmp(buf,"write\n")==0 ) {
        if(ifwrite == 1){
            if(writetype==3){writeF();};
            if(writetype==1){writeB();};
        }
        return 0;
    }
    
    if( strcmp(buf,"quit\n")==0 ) {
        if(ifwrite == 1){
            while(1){
                printf("Discard modifications (yes or no)?");
                getinput(writebuf,128,stdin);
                if( strcmp(writebuf,"yes\n")==0 ) {
                    freeTables();
                    fclose(pFile);
                    return 9;
                }
                if( strcmp(writebuf,"no\n")==0 ) {
                    return 0;
                }
            }
        }
        freeTables();
        fclose(pFile);
        return 9;
    }
    
    if( strcmp(buf,"history\n")==0 ) {
        printhistory();
        return 0;
    }
    
    if( strcmp(buf,"size\n")==0 ) {
        if(section==0){printf("Section text is %d bytes long\n",header.data[0]);}
        if(section==1){printf("Section rdata is %d bytes long\n",header.data[1]);}
        if(section==2){printf("Section data is %d bytes long\n",header.data[2]);}
        if(section==3){printf("Section sdata is %d bytes long\n",header.data[3]);}
        if(section==4){printf("Section sbss is %d bytes long\n",header.data[4]);}
        if(section==5){printf("Section bss is %d bytes long\n",header.data[5]);}
        if(section==6){printf("Section reltab is %d entries long\n",header.data[6]);}
        if(section==7){printf("Section reftab is %d entries long\n",header.data[7]);}
        if(section==8){printf("Section symtab is %d entries long\n",header.data[8]);}
        if(section==9){printf("Section strings is %d bytes long\n",header.data[9]);}
        return 0;
    }
    
    /// if repeat history
    if( sscanf(buf,"!%d",&repeat) == 1 ) {
        if(repeathistory(repeat)==-1){
            return 0;
        }
    }
    
    /// if section format
    if( sscanf(buf,"section %s",sect) == 1 ) {
        
        if( strcmp(sect,"text")==0 ){
            section = 0;
            printf("Now editing section text\n");
        }else if( strcmp(sect,"rdata")==0 ){
            section = 1;
            printf("Now editing section rdata\n");
        }else if( strcmp(sect,"data")==0 ){
            section = 2;
            printf("Now editing section data\n");
        }else if( strcmp(sect,"sdata")==0 ){
            if(header.data[3]==0){
                fprintf( stderr, "error:  cannot edit sdata section\n");
            }else{
                section = 3;
                printf("Now editing section sdata\n");
            }
        }else if( strcmp(sect,"sbss")==0 ){
            if(header.data[4]==0){
                fprintf( stderr, "error:  cannot edit sbss section\n");
            }else{
                section = 4;
                printf("Now editing section sbss\n");
            }
        }else if( strcmp(sect,"bss")==0 ){
            if(header.data[5]==0){
                fprintf( stderr, "error:  cannot edit sbss section\n");
            }else{
                section = 5;
                printf("Now editing section bss\n");
            }
        }else if( strcmp(sect,"reltab")==0 ){
            section = 6;
            printf("Now editing section reltab\n");
        }else if( strcmp(sect,"reftab")==0 ){
            section = 7;
            printf("Now editing section reftab\n");
        }else if( strcmp(sect,"symtab")==0 ){
            section = 8;
            printf("Now editing section symtab\n");
        }else if( strcmp(sect,"string")==0 ){
            section = 9;
            printf("Now editing section string\n");
        }else{
            fprintf( stderr, "error: '%s' is not a valid section name\n", sect);
            return 0;
        }
        return 0;
    }
    
    /// initilization
    strcpy(A0,"");strcpy(N0,"");strcpy(T0,"");strcpy(V0,"");
    A = -1; N=-1; T=-1; V=-1;
    
    /// Otherwise read A[:N][:T][=V] format
    ///sscanf(buf,"%[^,], %[^:]: %[^=]= %s %*[^\n]\n",A0,N0,T0,V0);
    sscanf(buf,"%[^,], %s %*[^\n]\n",A0,N0);
    sscanf(buf,"%*[^:]: %c %*[^\n]\n",T0);
    sscanf(buf,"%*[^,], %*[^:]: %c %*[^\n]\n",T0);
    sscanf(buf,"%*[^=]= %s %*[^\n]\n",V0);
    sscanf(buf,"%*[^,], %*[^=]= %s %*[^\n]\n",V0);
    
    /// A
    if(header.entry)
    {/// load module
        if( sscanf(A0,"%X",&A) == 1 ) {
            A1=A;
            ///printf("A:%d\n",A);
        }else{
            sscanf(A0,"%s",sect);
            fprintf( stderr, "error:  '%s' is not a valid address\n", sect);
            return 1;
        }
    }else{/// object module
        /// printf("A0:%s\n",A0); printf("N0:%s\n",N0);  printf("T0:%s\n",T0);  printf("V0:%s\n",V0);
        if( sscanf(A0,"%u",&A) == 1 ) {
            A1=A;
            ///printf("A:%d\n",A);
        }else{
            sscanf(A0,"%s",sect);
            fprintf( stderr, "error:  '%s' is not a valid address\n", sect);
            return 1;
        }
    
    }
    
    /// N
    if( sscanf(N0,"%u",&N) == 1 ) {
            ///printf("N:%d\n",N);
    }else{
        if(N0[0]){
        sscanf(N0,"%s",sect);
        fprintf( stderr, "error:  '%s' is not a valid count\n", sect);
        return 1;
        }
    }
    
    /// T
    if( T0[0] ) {
        /// printf("T0\n");
        if(strcmp(T0,"b")==0){
            T=1;
        }else if(strcmp(T0,"h")==0){
            T=2;
        }else if(strcmp(T0,"w")==0){
            T=3;
        }else{
            fprintf( stderr, "error:  '%s' is not a valid type\n", T0);
            return 1;
        }
    }
    
    /// V
    if( sscanf(V0,"%u",&V) == 1 ) {
        V = (int) V;
        ///printf("V:%d\n",V);
    }
    
    ///printf("A:%d\n",A); printf("N:%d\n",N);  printf("T:%d\n",T);  printf("V:%d\n",V);
    
    /// start editing
    if(header.entry)
    {/// load module
        load();
    }else{ /// object module
        object();
    }
    
    return 0;
}

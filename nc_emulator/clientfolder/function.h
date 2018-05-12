#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
size_t countStrLen(char * str){
    size_t c = 0;
    while(*str != '\0'){
        ++c;
        str++;
    }
    return c;
}

void printData(char * str, size_t numBytes){
    for (int i = 0; i < numBytes; i++){
        printf("%c", str[i]);    
    }
    printf("\n");
}
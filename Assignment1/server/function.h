#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
/*function that count the length of the string.*/
size_t countStrLen(char *str)
{
    size_t c = 0;
    while (*str != '\0')
    {
        ++c;
        str++;
    }
    return c;
}

/*function to print the data.*/
void printData(char *str, size_t numBytes)
{
    for (int i = 0; i < numBytes; i++)
    {
        printf("%c", str[i]);
    }
    printf("\n");
}
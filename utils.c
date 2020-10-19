#include "utils.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static int calcNumDigits(int num)
{
	if (num == 0)
		return 1;
	return 1 + calcNumDigits(num/10);
}

char* intToStr(int num) 
{
	int buffer = calcNumDigits(num) + 1;
	char* str = malloc(buffer * sizeof(char));
	if (str == NULL){
		return NULL;
	}
	sprintf(str, "%d", num);
	return str;
}

int strToInt(char* str) 
{
	if (!str){
		return 0;
	}
	int res = 0;
    for (int i = 0; str[i] != '\0'; ++i) {
        res = res * 10 + str[i] - '0'; 
	}
	free(str);
    return res; 
}

char* makeCopy (const char* str) 
{
	if (!str){
		return NULL;
	}
	char* str_copy = malloc(strlen(str)+1);
	if (!str_copy){
		return NULL;
	}
	strcpy(str_copy, str);
	return str_copy;
}
#ifndef UTILS_H_
#define UTILS_H_

/*
*   utility functions for area.c and election.c:
*
*   makeCopy		- recieves a string, allocates a copy to the string and returns a pointer to it 
*   intToStr		- recieves an int, allocates a string that stores the number and returns a pointer 
*   strToInt		- recieves a string and converts it to an int (frees the string)
*
*/

char* makeCopy(const char* str);
char* intToStr(int num);
int strToInt(char* str);

#endif /* UTILS_H_ */

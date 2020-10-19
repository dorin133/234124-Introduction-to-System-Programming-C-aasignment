#ifndef INFO_H_
#define INFO_H_

/*
* info -  an ADT that stores the id and name of an area
*         the type of the id is int, and the type of name is char* 
*
* The following functions are available:
*   infoCreate	- Create a new key with valid arguments.
*                 return the new key or NULL if a memory error occurred.
*   infoDestroy	- Delete an existing key and frees all resources.
*   infoCompare	- Return true if the current info has the same given id, false if not.
*   infoGetId   - Return the key element of a given key.
*   infoGetName - Return the data of the given key.
*/

typedef struct info_t* Info; 

#include <stdbool.h>
#include <string.h>

Info infoCreate(int id,const char* name);
void infoDestroy(Info info);
bool infoCompare(Info info, int id_compare);
int infoGetId(Info info);
char* infoGetName(Info info);

#endif
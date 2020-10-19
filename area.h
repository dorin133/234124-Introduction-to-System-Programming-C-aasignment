#ifndef AREA_H_
#define AREA_H_

#include <stdbool.h>
#include <string.h>

/*
* Implements the area ADT.
* The area ADT contains the fields: 
* info -  an ADT that stores the id and name of the area
*         the type of the id is int, and the type of name is char*
* votes - a map that stores the votes for the tribes in the area in the following format:
*         key = tribe id, value = num of votes for the tribe
*         the type of the key and the value is string (char *)
*
*   areaCreate		- Creates a new empty area
*   areaDestroy		- Deletes an existing area and frees all resources
*   areaExists		- checks if an area with the given id exists already
*   areaGetId		- Returns the id of a given area
*   areaGetName	    - Returns the name of a given area
*   areaGetMap		- Returns the votes map of a given area. 
*   areaGetNumVotes - Returns the number of votes for a given tribe from the area.
*   areaAddRemoveVotes - Adds or removes votes for a given tribe from the area.
*   areaRemoveTribe	- removes votes for the tribe in the area 
*   areaFindMaxVotes- returns the id of the tribe with the most votes in the area
*                     in case there are no votes, returns the min tribe id
*	areaIsEmptyVotes - checks if there are no votes in the area yet
*/

typedef struct Area_t* Area;

typedef enum AreaResult_t {
    AREA_OUT_OF_MEMORY,
    AREA_SUCCESS, 
    AREA_NULL_ARGUMENT,
} AreaResult;

Area areaCreate(int area_id, const char* area_name);
void areaDestroy(Area area);
bool areaExists(Area area, int area_compare);
int areaGetId(Area area);
char* areaGetName(Area area);
Map areaGetMap(Area area);
bool areaExists(Area area, int area_compare);
int areaGetNumVotes(Area area, int tribe_id);
AreaResult areaAddRemoveVotes(Area area, int tribe_id_in, int num_of_votes);
AreaResult areaRemoveTribe(Area area, char* tribe_id);
char* areaFindMaxVotes(Area area, char* min_tribe_id);
bool areaIsEmptyVotes(Area area);

#endif /* AREA_H_ */

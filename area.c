#include "mtm_map/map.h"
#include "area.h"
#include "info.h"
#include "utils.h"
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

#define MAX(a,b) (((a)>(b))?(a):(b))
#define MEMORY_ERROR -1

struct Area_t {
    Info info;
    Map votes; //votes: key=tribe_id, value=num of votes
};

//recieves 2 tribe ids and numbers of votes, and returns the id of the tribe that has the most votes
static char* maxVotesId(char* first_id, char* first_num_votes, char* second_id, char* second_num_votes) 
{
	char* first_num_votes_copy = makeCopy(first_num_votes);
	if(!first_num_votes_copy) {
		return NULL;
	}
	char* second_num_votes_copy = makeCopy(second_num_votes);
	if(!second_num_votes_copy) {
		free(first_num_votes_copy);
		return NULL;
	}
	if (strcmp(first_num_votes, second_num_votes) == 0) { 
		free(first_num_votes_copy);
		free(second_num_votes_copy);
		char* first_id_copy = makeCopy(first_id);		
		char* second_id_copy = makeCopy(second_id);
		if(!first_id_copy || !second_id_copy) {
			if(first_id_copy != NULL) {
				free(second_id_copy);
			}
			return NULL;
		}
		return strToInt(first_id_copy) < strToInt(second_id_copy) ? first_id : second_id;
	}
	return strToInt(first_num_votes_copy) > strToInt(second_num_votes_copy) ? first_id : second_id;
}

Area areaCreate(int area_id, const char* area_name) 
{
	Area new_area = malloc(sizeof(*new_area));
	if(!new_area) {
		return NULL;
	}
	//add info:
	new_area->info = infoCreate(area_id, area_name);
	if (!new_area->info){
        free(new_area);
        return NULL;
    }
	//create empty map for the votes:
	new_area->votes = mapCreate();
	if (!new_area->votes){
		infoDestroy(new_area->info);
		free(new_area);
        return NULL;
	}
	return new_area;
}

void areaDestroy(Area area) 
{
	infoDestroy(area->info);
	mapDestroy(area->votes);
    free(area);
}

int areaGetId(Area area)
{
	return infoGetId(area->info);
}

char* areaGetName(Area area)
{
	return infoGetName(area->info);
}

Map areaGetMap(Area area)
{
	return area->votes;
}

bool areaExists(Area area, int area_compare) 
{
	if (infoCompare(area->info, area_compare)) 
		return true;
	return false;
}

bool areaIsEmptyVotes(Area area) 
{
	if (mapGetSize(area->votes) <= 0){
		return true;
	}
	return false;
}

int areaGetNumVotes(Area area, int tribe_id) 
{
	char* tribe_id_str = intToStr(tribe_id);
	if (!tribe_id_str){
		return MEMORY_ERROR;
	}
	char* num_votes_str = mapGet(area->votes, tribe_id_str);
	free(tribe_id_str); 
	if (!num_votes_str){ //the tribe does not exist in the votes map yet, which translates to zero votes
		return 0;
	}
	char* num_votes_copy = makeCopy(num_votes_str); 
	if(!num_votes_copy){
		return MEMORY_ERROR; 
	}
	return strToInt(num_votes_copy); 
}

AreaResult areaAddRemoveVotes(Area area, int tribe_id, int num_votes) 
{
	int current_num_votes = areaGetNumVotes(area, tribe_id);
	if (current_num_votes == -1){
		return AREA_OUT_OF_MEMORY;
	}
	//user wants to subtract votes, but the tribe doesn't have any votes yet -> do nothing and return success.
	if ((current_num_votes == 0) && (num_votes < 0)){
		return AREA_SUCCESS; 
	}
	char* num_votes_str = intToStr(MAX(current_num_votes + num_votes, 0));
	if (!num_votes_str) {
		return AREA_OUT_OF_MEMORY;
	}
	char* tribe_id_str = intToStr(tribe_id);
	if (!tribe_id_str) {
		free(num_votes_str);
		return AREA_OUT_OF_MEMORY;		
	}
    MapResult result = mapPut(area->votes, tribe_id_str, num_votes_str);
	free(num_votes_str);
	free(tribe_id_str); 
	return result == MAP_SUCCESS ? AREA_SUCCESS : AREA_OUT_OF_MEMORY;  
}  

AreaResult areaRemoveTribe(Area area, char* tribe_id)
{
	if (!area || !tribe_id){
		return AREA_NULL_ARGUMENT;
	}
	MapResult result = mapRemove(area->votes, tribe_id);
	if(result == MAP_NULL_ARGUMENT) {
		return AREA_NULL_ARGUMENT;
	}
	assert(result == MAP_SUCCESS || result == MAP_ITEM_DOES_NOT_EXIST);
	return AREA_SUCCESS;
}

//returns the id of the winning tribe as a string
char* areaFindMaxVotes(Area area, char* min_tribe_id) 
{ 
	if(!area){
		return NULL;
	}
	if (areaIsEmptyVotes(area)){
		return min_tribe_id;
	}
	char* current_num_votes = NULL; 
	char* tribe_max_id = mapGetFirst(area->votes); //initialize max id - first tribe in this map
	char* max_votes = mapGet(area->votes, tribe_max_id); 
	MAP_FOREACH(current_tribe_id, area->votes) {
		current_num_votes = mapGet(area->votes, current_tribe_id);				
		char* new_max_id = maxVotesId(tribe_max_id , max_votes, current_tribe_id, current_num_votes);
		if(!new_max_id) {
			return NULL;
		}
		//if the tribe with the max votes has changed, update its number of votes:
		if(strcmp(tribe_max_id, new_max_id) != 0) { 
			tribe_max_id = new_max_id;
			max_votes = current_num_votes;
		}			
	}
	if (strcmp(max_votes,"0") == 0){
		return min_tribe_id;
	}
	return tribe_max_id;
}
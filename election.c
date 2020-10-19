//this is the hand in version
#include "mtm_map/map.h"
#include "election.h"
#include "area.h"
#include "info.h"
#include "utils.h"
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#define AREA 1
#define TRIBE 2
#define MEMORY_ERROR -1
#define VALID_VOTES 1
#define INITIAL_SIZE 2

struct election_t{
    Map tribes;
    Map votes; //key: area_id,area_name. value: tribe_id1:votes1,tribe_id2:votes2...
};

//converts from info ADT to string, in order to store in the map. output format: area_id:area_name
static char* infoToString(Area area)
{
    assert(area);
    int area_id_int = areaGetId(area);
    char* area_id = intToStr(area_id_int);
    if (!area_id){
        return NULL;
    }
    char* area_name = areaGetName(area);
    int length = strlen(area_id) + strlen(area_name) + 2;//extra 1 for ':' + 1 for '\0'
    char* key = malloc(length * sizeof(char));
    if(!key) {
        free(area_id);
        return NULL;
    }
    strcpy(key, area_id);
    strcat(key, ":");
    strcat(key, area_name);
    free(area_id);
    return key;
}

//converts the votes map in the area type to string, in order to store in the map
//output format: tribe_id1:num_votes1,tribe_id2:num_votes2,...,
static char* votesToString(Area area)
{
    assert(area);
    Map votes = areaGetMap(area);
    char* value = malloc(INITIAL_SIZE * sizeof(char));
    if(!value) {
        return NULL;
    }
    strcpy(value,",");
    MAP_FOREACH(current_tribe, votes){
        char* tribe_id_str = makeCopy(current_tribe);
        if (!tribe_id_str){
            free(value);
            return NULL;
        }
        int tribe_id = strToInt(tribe_id_str); 
        int num_votes = areaGetNumVotes(area, tribe_id);
        char* num_votes_str = intToStr(num_votes);
        if (!num_votes_str || (num_votes == MEMORY_ERROR)){
            free(value);
            return NULL;
        }
        //allocate destination string
        int value_length = 0;
        if (value){
            value_length = strlen(value);
        }
        int new_length = value_length + strlen(current_tribe) + strlen(num_votes_str) + 3; //extra 3: ":" + "," + "\0"
        value = realloc(value, new_length * sizeof(char));
        if (!value) {
            free(num_votes_str);
            free(value);
            return NULL;
        }
        //insert to string: (assuming the last entry has "," at the end)
        strcat(value, current_tribe);
        strcat(value, ":");
        strcat(value, num_votes_str);
        strcat(value, ",");
        free(num_votes_str);
    }
    return value; 
}

//recieves a string in the format: area_id,area_name and converts it to Info ADT (in order to store as a field in Area type)
static Info breakToken(char* str) 
{
    char* copy = makeCopy(str);
    char* area_id_str = makeCopy(strtok(copy, ":"));
    char* area_name = makeCopy(strtok(NULL, ":"));
    if (!area_id_str || !area_name){
        return NULL;
    }
    int area_id = strToInt(area_id_str);
    Info info = infoCreate(area_id, area_name);
    free(copy);
    free(area_name);
    if(!info) {
        return NULL;
    }
    return info;
}

//recieves an area and a string in the following format: tribe_id1:num_votes1,tribe_id2:num_votes2,...,
//translates the string to votes map that would be stored in the area. 
//map format: key=tribe id, value = number of votes
static Area stringToMap(Area area, char* value_str) 
{
    char* copy = makeCopy(value_str); //make sure to free this later
    char* token = strtok(copy, ",");
    while(token != NULL)
    { //if token_tribe_id != null, then token_num_of_votes definitely exists 
        int i;
	    int length = strlen(token);
	    for(i=0; i < length; i++) {
		    if (token[i] == ':'){
			    break;
            }
	    }
	    char* tribe_id = malloc((i+1) * sizeof(char));
        char* num_votes_str = malloc((length - i + 1) * sizeof(char));
        if(!tribe_id || !num_votes_str) {
            free(copy);
            areaDestroy(area);
            if(tribe_id) {
                free(tribe_id);
            }
            return NULL;
        }
	    memcpy(tribe_id, token, i);
	    tribe_id[i] = '\0';
	    memcpy(num_votes_str, (token + i+1), length-i);
        int num_votes = strToInt(num_votes_str);
        int tribe_id_num = strToInt(tribe_id);
        AreaResult result = areaAddRemoveVotes(area, tribe_id_num, num_votes); 
        if(result != AREA_SUCCESS) {
            free(copy);
            areaDestroy(area);
            return NULL;
        }
        token = strtok(NULL, ",");
    } 
    free(copy);
    return area;
}

//create Area ADT that stores the information from the key and value string, as they are kept in the votes map
static Area createAreaFromString(char* key_str, char* value_str) 
{
    Info area_info = breakToken(key_str); 
    if (!area_info) {
        return NULL;
    }
    Area area = areaCreate(infoGetId(area_info), infoGetName(area_info));
    infoDestroy(area_info);
    if(!area) {
        return NULL;
    }
    if (!value_str) { //no votes in area - finished converting
        return area;
    }
    return stringToMap(area, value_str);
}

//checks if an area/tribe id is valid
static ElectionResult isValidId(int id_num)
{
    if (id_num < 0) {
        return ELECTION_INVALID_ID;
    }
    return ELECTION_SUCCESS;
}

//checks if the number of votes is valid
static ElectionResult isValidVotes(int votes_num)
{
    if (votes_num <= 0) {
        return ELECTION_INVALID_VOTES;
    }
    return ELECTION_SUCCESS;   
}

//checks if an area/tribe name is valid
static bool isNameValid(const char* name)
{
    assert(name);
    for (int i = 0 ; i < strlen(name) ; i++) {
        if ((name[i] < 'a') || (name[i] > 'z')){
            if (name[i] != ' ') {
                return false;
            }
        }
    }
    return true;
}

//checks if an area with the given id already exists 
static ElectionResult areaContains(Election election, int id) 
{
    MAP_FOREACH(current_area_info, election->votes) {
        Area current_area = createAreaFromString(current_area_info, NULL);
        if(!current_area) {
            return ELECTION_OUT_OF_MEMORY;
        }
        if(areaExists(current_area, id)) {
            areaDestroy(current_area);          
            return ELECTION_AREA_ALREADY_EXIST;
        }
        areaDestroy(current_area);
    }
    return ELECTION_AREA_NOT_EXIST;
}

//inserts an area and its votes to the votes map of the election
static ElectionResult areaPut(Election election, Area area) 
{   
    char* area_info = infoToString(area);
    if (!area_info){
        areaDestroy(area);
        return ELECTION_OUT_OF_MEMORY;
    }
    MapResult result = MAP_ERROR;
    if(!areaIsEmptyVotes(area)) { 
        char* votes_str = votesToString(area); 
        if (!votes_str) {
            areaDestroy(area);
            free(area_info);
            return ELECTION_OUT_OF_MEMORY;           
        }
        result = mapPut(election->votes, area_info, votes_str);
        free(votes_str);
    }
    else { //if there are no votes insert empty string as the value
        result = mapPut(election->votes, area_info, "");
    }
    free(area_info);
    areaDestroy(area);
    return (result == MAP_SUCCESS) ? ELECTION_SUCCESS : ELECTION_OUT_OF_MEMORY;
}

// Check the input of the given variables and return errors/success as instructed.
static ElectionResult electionCheckInput(Election election, int id, int area_or_tribe) 
{ 
    if (!election){
        return ELECTION_NULL_ARGUMENT;
    }
    if (isValidId(id) == ELECTION_INVALID_ID) {
        return ELECTION_INVALID_ID;
    }
    //check if area/tribe already exists:
    ElectionResult exist = ELECTION_SUCCESS;
    if (area_or_tribe == TRIBE) {
        char* tribe_id_str = intToStr(id);
        if(!tribe_id_str){
            return ELECTION_OUT_OF_MEMORY;
        }
        exist = mapContains(election->tribes, tribe_id_str) ? ELECTION_TRIBE_ALREADY_EXIST : ELECTION_TRIBE_NOT_EXIST;
        free(tribe_id_str);
    }
    else {        
        exist = areaContains(election, id);
    }
    return exist; 
}

// Check the input of the given variables and return errors/success as instructed.
static ElectionResult electionCheckAddRemove(Election election, int area_id, int tribe_id, int num_of_votes) 
{ //input is valid: 
    if (!election){
        return ELECTION_NULL_ARGUMENT;
    }
    if ((isValidId(area_id) == ELECTION_INVALID_ID) || (isValidId(tribe_id) == ELECTION_INVALID_ID)) {
        return ELECTION_INVALID_ID;
    }
    if (isValidVotes(num_of_votes) == ELECTION_INVALID_VOTES){
        return ELECTION_INVALID_VOTES;    
    }
    //check if area/tribe already exists:
    if (areaContains(election, area_id) != ELECTION_AREA_ALREADY_EXIST){
        return ELECTION_AREA_NOT_EXIST;
    }
    char* tribe_id_str= intToStr(tribe_id);
    if(!tribe_id_str) {
        return ELECTION_OUT_OF_MEMORY;
    }
    if (!mapContains(election->tribes, tribe_id_str)){
        free(tribe_id_str);
        return ELECTION_TRIBE_NOT_EXIST;
    }
    free(tribe_id_str);
    return ELECTION_SUCCESS;
}

/* Check if Adding /Subtracting the number of votes for the current tribe (tribe_id) 
   from the current area (area_id) ended with status ELECTION_SUCCESS */
static ElectionResult resAddRemoveVotes(Election election, int area_id, int tribe_id, int num_of_votes)
{
    MAP_FOREACH(area_info, election->votes){
        Area temp_area = createAreaFromString(area_info, mapGet(election->votes, area_info));
        //we assume that the area exists, so if NULL is returned it's due to a memory problem
        if (!temp_area){
            return ELECTION_OUT_OF_MEMORY;
        }
        if (areaExists(temp_area, area_id)){ //we found the area we were looking for
            AreaResult result = areaAddRemoveVotes(temp_area, tribe_id, num_of_votes);
            if (result == AREA_SUCCESS){
                return areaPut(election, temp_area);
            }
            else {
                areaDestroy(temp_area);
                return ELECTION_OUT_OF_MEMORY;
            }
        }
        //if this isn't the correct area, we destroy it and move on to the next
        areaDestroy(temp_area);
    }
    //assuming the tribe and area do exist, so we never reach here
    assert(0);
    return ELECTION_ERROR;
}

//return (in char*) the id of the tribe with the lowest number of votes within the current area.
static char* findMinTribeId(Map tribes) 
{
    assert(tribes);
    char* min_tribe_id = mapGetFirst(tribes);
    MAP_FOREACH(current_tribe, tribes){
        char* min_tribe_id_copy = makeCopy(min_tribe_id);
        if(!min_tribe_id_copy) {
            return NULL;
        }
        char* current_tribe_copy = makeCopy(current_tribe);
        if(!current_tribe_copy) {
            free(min_tribe_id_copy);
            return NULL;
        }
        min_tribe_id = strToInt(min_tribe_id_copy) < strToInt(current_tribe_copy) ? min_tribe_id : current_tribe;
    }
    return min_tribe_id;
}
//end of static functions

Election electionCreate()
{
    Election new_election = malloc(sizeof(*new_election));
	if(!new_election){
		return NULL;
    }
    new_election->tribes = mapCreate();
    if (!new_election->tribes) {
        free(new_election);
        return NULL;
    }
    new_election->votes = mapCreate(); 
    if (!new_election->votes){
        mapDestroy(new_election->tribes);
        free(new_election);
        return NULL;
    }
    return new_election;
}

void electionDestroy(Election election)
{
    if (election == NULL){
        return;
    }
    mapDestroy(election->tribes);
    mapDestroy(election->votes);
    free(election);
}

ElectionResult electionAddTribe (Election election, int tribe_id, const char* tribe_name) 
{
    if (!tribe_name){
        return ELECTION_NULL_ARGUMENT;
    }
    ElectionResult check = electionCheckInput(election, tribe_id, TRIBE);
    if (check != ELECTION_TRIBE_NOT_EXIST) {
        return check;
    } 
    if (!isNameValid(tribe_name)){
        return ELECTION_INVALID_NAME; 
    }
    char* str_tribe_id = intToStr(tribe_id);
    if (!str_tribe_id) {
        return ELECTION_OUT_OF_MEMORY;
    }
    MapResult result = mapPut(election->tribes, str_tribe_id, tribe_name);
    if (result != MAP_SUCCESS) {
        free(str_tribe_id);
        if (result == MAP_NULL_ARGUMENT){
            return ELECTION_NULL_ARGUMENT;
        }
        if (result == MAP_OUT_OF_MEMORY){
            return ELECTION_OUT_OF_MEMORY;
        }
    }
    free(str_tribe_id);
    return ELECTION_SUCCESS;
}
 
ElectionResult electionAddArea(Election election, int area_id, const char* area_name) 
{
    if (!area_name){
        return ELECTION_NULL_ARGUMENT;
    }
    ElectionResult check = electionCheckInput(election, area_id, AREA);
    if (check != ELECTION_AREA_NOT_EXIST){
        return check;
    }
    Area new_area = NULL;
    if (strcmp(area_name, " ") == 0){
        new_area = areaCreate(area_id, area_name);
        if (!new_area){
            return ELECTION_OUT_OF_MEMORY;
        }
    }
    else{
        if (!isNameValid(area_name)){
            return ELECTION_INVALID_NAME; 
        }
        new_area = areaCreate(area_id, area_name);
        if (!new_area){
            return ELECTION_OUT_OF_MEMORY;
        }
    }
    return areaPut(election, new_area);
}

char* electionGetTribeName (Election election, int tribe_id)
{
    if (!election){
        return NULL;
    }
    char* tribe_id_str = intToStr(tribe_id);
    if(!tribe_id_str) {
        return NULL;
    }
    if (!(mapContains(election->tribes, tribe_id_str))) {
        free(tribe_id_str);
        return NULL;
    } 
    char* result = makeCopy(mapGet(election->tribes, tribe_id_str));
    if (!result){
        free(tribe_id_str);
        return NULL;
    }
    free(tribe_id_str);
    return result; 
} 

ElectionResult electionSetTribeName (Election election, int tribe_id, const char* tribe_name)
{
    if (!tribe_name){
        return ELECTION_NULL_ARGUMENT;
    }
    ElectionResult check = electionCheckInput(election, tribe_id, TRIBE);
    if (check != ELECTION_TRIBE_ALREADY_EXIST) {
        return check;
    }
    if (!isNameValid(tribe_name)){
        return ELECTION_INVALID_NAME; 
    }
    char* id_str = intToStr(tribe_id);
    if (!id_str){
        return ELECTION_OUT_OF_MEMORY;
    }
    if (mapPut(election->tribes, id_str, tribe_name) != MAP_SUCCESS) {
        free(id_str);
        return ELECTION_OUT_OF_MEMORY; 
    }
    free(id_str);
    return ELECTION_SUCCESS;
}

//recursive deletion of the tribe from all areas
static ElectionResult electionRemoveTribeVotes(Election election, char* tribe_id_str, int tribe_id)
{
    bool deleted = false; 
    MAP_FOREACH(current_area_info, election->votes) {
        Area current_area = createAreaFromString(current_area_info, mapGet(election->votes, current_area_info));
        if(!current_area) {
            return ELECTION_OUT_OF_MEMORY;
        }
        int num_of_votes = areaGetNumVotes(current_area, tribe_id);
        if(num_of_votes == MEMORY_ERROR) {
            free(tribe_id_str);
            areaDestroy(current_area);
            return ELECTION_OUT_OF_MEMORY;
        }
        if(num_of_votes == 0) {
            areaDestroy(current_area);
            continue;
        }
        if(areaRemoveTribe(current_area, tribe_id_str) != AREA_SUCCESS) {
            free(tribe_id_str);
            areaDestroy(current_area);
            return ELECTION_NULL_ARGUMENT;
        }
        if(areaPut(election, current_area) == ELECTION_OUT_OF_MEMORY) {
            return ELECTION_OUT_OF_MEMORY;
        }
        deleted = true;
    }
    if (deleted){ 
        return electionRemoveTribeVotes(election, tribe_id_str, tribe_id);
    }
    free(tribe_id_str);
    return ELECTION_SUCCESS; 
}

ElectionResult electionRemoveTribe (Election election, int tribe_id) 
{
    ElectionResult check = electionCheckInput(election, tribe_id, TRIBE);
    if (check != ELECTION_TRIBE_ALREADY_EXIST){
        return check;
    }
    char* tribe_id_str = intToStr(tribe_id);
    if (!tribe_id_str){
        return ELECTION_OUT_OF_MEMORY;
    }
    //remove tribe from the tribes map
    if(mapRemove(election->tribes, tribe_id_str) != MAP_SUCCESS) {
        free(tribe_id_str);
        return ELECTION_OUT_OF_MEMORY;
    }
    //remove tribe from the votes map:
    return electionRemoveTribeVotes(election, tribe_id_str, tribe_id);
}

ElectionResult electionRemoveAreas(Election election, AreaConditionFunction should_delete_area) 
{
    if(!election || !should_delete_area) {
        return ELECTION_NULL_ARGUMENT;
    }
    if (mapGetSize(election->votes) == 0){
        return ELECTION_SUCCESS;
    }
    Map votes_copy = mapCopy(election->votes);
    if (!votes_copy){
        return ELECTION_OUT_OF_MEMORY;
    }
    MAP_FOREACH(current_area_info, election->votes) {
        Area current_area = createAreaFromString(current_area_info, mapGet(election->votes, current_area_info)); 
        if(!current_area) {
            mapDestroy(votes_copy);
            return ELECTION_OUT_OF_MEMORY;
        }
        if(should_delete_area(areaGetId(current_area))) {
            if(mapRemove(votes_copy, current_area_info) != MAP_SUCCESS) {
                mapDestroy(votes_copy);
                areaDestroy(current_area);               
                return ELECTION_OUT_OF_MEMORY;
            }                   
        }
        areaDestroy(current_area);
    }
    mapDestroy(election->votes);
    election->votes = votes_copy;    
    return ELECTION_SUCCESS;
}

ElectionResult electionAddVote (Election election, int area_id, int tribe_id, int num_of_votes)
{    
    ElectionResult check = electionCheckAddRemove(election, area_id, tribe_id, num_of_votes);
    if (check != ELECTION_SUCCESS){
        return check;
    }
    return resAddRemoveVotes(election, area_id, tribe_id, num_of_votes);
}

ElectionResult electionRemoveVote (Election election, int area_id, int tribe_id, int num_of_votes)
{
    ElectionResult check = electionCheckAddRemove(election, area_id, tribe_id, num_of_votes);
    if (check != ELECTION_SUCCESS){
        return check;
    }
    return resAddRemoveVotes(election, area_id, tribe_id, (-1)*num_of_votes);
}

Map electionComputeAreasToTribesMapping (Election election) 
{
    Map result = mapCreate();
    if (!election  || !result){
        return NULL;
    }
    if ((mapGetSize(election->tribes) <= 0) || !election->votes){
        return result;
    }
    char* min_tribe_id = findMinTribeId(election->tribes);
    if(!min_tribe_id) {
        mapDestroy(result);
        return NULL;
    }
    MAP_FOREACH(current_area_info, election->votes){
        Area temp_area = createAreaFromString(current_area_info, mapGet(election->votes, current_area_info));
        if (!temp_area){
            mapDestroy(result);
            return NULL;
        }
        char* tribe_max_id = areaFindMaxVotes(temp_area, min_tribe_id); //findmax returns the min if there are no votes
        char* area_id_str = intToStr(areaGetId(temp_area));
        if (!tribe_max_id || !area_id_str){
            areaDestroy(temp_area);
            mapDestroy(result);
            return NULL;
        }
        MapResult map_put_result = mapPut(result, area_id_str, tribe_max_id); 
        free(area_id_str);
        areaDestroy(temp_area);
        assert(map_put_result!=MAP_NULL_ARGUMENT);
        if (map_put_result != MAP_SUCCESS) {
            mapDestroy(result);
            return NULL;
        }           
    }
    return result;
}
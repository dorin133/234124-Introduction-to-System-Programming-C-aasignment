#include "info.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

struct info_t{
    int id;    
    char* name;
};

Info infoCreate(int id, const char* name)
{ 
    Info new_info = malloc(sizeof(*new_info)); 
    if (new_info == NULL){
        return NULL;
    }
    char* info_name = malloc(strlen(name)+1); 
    if (!info_name){
        free(new_info);
        return NULL; 
    }
    strcpy(info_name, name);
    new_info->name = info_name;
    new_info->id = id;
    return new_info;
}

void infoDestroy(Info info)
{
    if (!info){
        return;
    }
    free(info->name);
    free(info);
}

bool infoCompare(Info info, int id_compare)
{
    return (info->id == id_compare);
}

int infoGetId(Info info)
{
    return info->id;
}

char* infoGetName(Info info)
{
    return info->name;
}

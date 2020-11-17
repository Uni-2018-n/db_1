#ifndef __HP_H__
#define __HP_H__

#include "Record.h"

#define MAX_ATTR_NAME_SIZE 30

struct HP_info
{
    char type[3]; // HP or HT
    int fileDesc;
    char attrType;
    char attrName[MAX_ATTR_NAME_SIZE];
    int attrLength; 
};

int HP_CreateFile(const char *fileName, char attrType, const char* attrName, int attrLength);
HP_info* HP_OpenFile(const char *fileName);
int HP_CloseFile(HP_info* header_info);
int HP_InsertEntry(HP_info header_info, Record record);
int HP_DeleteEntry(HP_info header_info, void *value);
int HP_GetAllEntries(HP_info header_info, void *value);

#endif // __HP_H__
#ifndef __HP_H__
#define __HP_H__

#include "Record.h"

#define MAX_ATTR_NAME_SIZE 15

struct HP_info
{
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
int ReadNumOfRecords(void *block);
void WriteNumOfRecords(void* block, int recordNumber);
int ReadNextBlockAddr(void* block);
void WriteNextBlockAddr(void* block, int blockNumber);
void ReadRecord(void* block, int recordNumber, Record* record);
void WriteRecord(void* block, int recordNumber, const Record* record);
void* InitBlock(HP_info* header_info, int blockNumber);
int IsBlockEmpty(int file_desc);
void ReplaceWithLastRecord(int pos, void* block);
int InitBlock(HP_info* header_info, int blockNumber, void** block);
int IsKeyInBlock(Record* record, void* block);
int AssignKeyToRecord(Record* record, void* value, char key_type);

// TODO: Remove this.
void PrintAllEntries(HP_info header_info);

#endif // __HP_H__
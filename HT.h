#ifndef __HT_H__
#define __HT_H__

#include "Record.h"
#define MAX_ATTR_NAME_SIZE 15
typedef struct {
  int fileDesc;
  char ht[3];
  char attrType;
  char attrName[MAX_ATTR_NAME_SIZE];
  int attrLength;
  long int numBuckets;
} HT_info;

int HT_CreateIndex(const char *fileName, const char attrType, const char* attrName, const int attrLength, const int buckets);

HT_info* HT_OpenIndex(char *fileName);

int HT_CloseIndex(HT_info* header_info);

int HT_InsertEntry(HT_info header_info, Record record);

int HT_DeleteEntry(HT_info header_info, void *value);

int HT_GetAllEntries( HT_info header_info, void *value);

int HT_function(int* value, int buckets);

int HT_function(char* value, int buckets);

int HT_HP_InsertEntry(HT_info* header_info, Record* record, int heap);
int HT_HP_DeleteEntry(HT_info* header_info, void* value, int heap_address);
int HT_HP_GetAllEntries(HT_info* header_info, void* value, int heap_addr);

int ReadNumOfRecords(void *block);
void WriteNumOfRecords(void* block, int recordNumber);
int ReadNextBlockAddr(void* block);
void WriteNextBlockAddr(void* block, int blockNumber);
void ReadRecord(void* block, int recordNumber, Record* record);
void WriteRecord(void* block, int recordNumber, const Record* record);
int IsBlockEmpty(int file_desc);
void ReplaceWithLastRecord(int pos, void* block);
int InitBlock(int fileDesc, void** block);
int IsKeyInBlock(Record* record, void* block);
int AssignKeyToRecord(Record* record, void* value, char key_type);

#endif

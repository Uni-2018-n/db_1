#include "HT.h"
#include "BF.h"
#include "HP.h"
#include <cstring>
#include <iostream>
#define MAX_RECORDS_IN_BLOCK ((BLOCK_SIZE - 2 * (int) sizeof(int)) / (int) sizeof(Record))

int HT_CreateIndex(const char *fileName, const char attrType, const char* attrName, const int attrLength, const int buckets){
  if(strlen(attrName) > MAX_ATTR_NAME_SIZE-1){
    printf("attrName too big\n");
    return -1;
  }
  BF_Init();

  BF_CreateFile(fileName);

  int file = BF_OpenFile(fileName);

  BF_AllocateBlock(file);

  void *block;
  BF_ReadBlock(file, 0, &block);
  memcpy((char *)block, "HT", 3);
	memcpy((char *)block + 3, &attrType, sizeof(char));
	memcpy((char *)block + 3 + sizeof(char), attrName, strlen(attrName) + 1);
	memcpy((char *)block + 3 + sizeof(char) + MAX_ATTR_NAME_SIZE , &attrLength, sizeof(int));
  memcpy((char *)block + 3 + sizeof(char) + MAX_ATTR_NAME_SIZE + sizeof(int), &buckets, sizeof(int));


  for(int i=0;i<buckets;i++){
    int temp =0;
    memcpy((char *)block + 3 + sizeof(char) + MAX_ATTR_NAME_SIZE + sizeof(int) + sizeof(int) + sizeof(int)*i, &temp, sizeof(int));
  }

  BF_WriteBlock(file, 0);

  BF_CloseFile(file);

  return 0;
}

HT_info* HT_OpenIndex(char *fileName){
  int file = BF_OpenFile(fileName);
  if(file <0){
    printf("error open file\n");
    return NULL;
  }
  HT_info* temp = new HT_info;
  temp->fileDesc = file;

  void* block;
  BF_ReadBlock(file, 0, &block);
  char* ht = new char[3];
  memcpy(ht, (char *)block, 3);
  if(strcmp(ht, "HT") != 0){
    printf("error block\n");
    return NULL;
  }
  memcpy(&(temp->attrType), (char *)block+3, sizeof(char));
  memcpy(&(temp->attrName), (char *)block + 3+1, MAX_ATTR_NAME_SIZE);
  memcpy(&(temp->attrLength), (char *)block + 3+1 + MAX_ATTR_NAME_SIZE, sizeof(int));
  memcpy(&(temp->numBuckets), (char *)block + 3 + 1 + MAX_ATTR_NAME_SIZE + sizeof(int), sizeof(long int));

  return temp;
}

int HT_CloseIndex(HT_info* header_info){
  int temp =BF_CloseFile(header_info->fileDesc);
  delete header_info;
  return temp;
}

int HT_InsertEntry(HT_info header_info, Record record){
  int h = HT_function(&record.id, header_info.numBuckets);
  int startup = 3+ sizeof(char) + MAX_ATTR_NAME_SIZE + sizeof(int) + sizeof(int);
  void* block;
  BF_ReadBlock(header_info.fileDesc, 0, &block);

  int heap;
  int i;
  for(i=0;i<header_info.numBuckets; i++){
    if(i==h){
        memcpy(&heap, (char *)block + startup + sizeof(int)*i, sizeof(int));
        break;
    }
  }

  int new_heap_addr = HT_HP_InsertEntry(&header_info, &record, heap);
  if (new_heap_addr == -1)
    return -1;

  // printf("new_heap_addr: %d\n", new_heap_addr);
  if (new_heap_addr != heap){
    memcpy((char*)block+startup+sizeof(int)*i, &new_heap_addr, sizeof(int));
    BF_WriteBlock(header_info.fileDesc, 0);
  }


  // this tests if hash_table's addresses is working (check if something changes from 0)
  // for(i=0;i<header_info.numBuckets; i++){
  //       memcpy(&heap, (char *)block + startup + sizeof(int)*i, sizeof(int));
  //       printf("hash table: %d\n", heap);
  // }
  return 0;
}

int HT_DeleteEntry(HT_info header_info, void *value){
  int h;
  if(header_info.attrType == 'i'){
    h = HT_function((int*)value, (int)header_info.numBuckets);
  }else{
    h = HT_function((char*)value, (int)header_info.numBuckets);
  }


  int startup = 3+ sizeof(char) + MAX_ATTR_NAME_SIZE + sizeof(int) + sizeof(int);
  void* block;
  BF_ReadBlock(header_info.fileDesc, 0, &block);

  int heap;
  int i;
  for(i=0;i<header_info.numBuckets; i++){
    if(i==h){
        memcpy(&heap, (char *)block + startup + sizeof(int)*i, sizeof(int));
        break;
    }
  }

  if (HT_HP_DeleteEntry(&header_info, value, heap) != 0){
    return -1;
  }

  return 0;
}

int HT_GetAllEntries(HT_info header_info, void *value){
  int h;
  if(header_info.attrType == 'i'){
    h = HT_function((int*)value, (int)header_info.numBuckets);
  }else{
    h = HT_function((char*)value, (int)header_info.numBuckets);
  }

  int startup = 3+ sizeof(char) + MAX_ATTR_NAME_SIZE + sizeof(int) + sizeof(int);
  void* block;
  BF_ReadBlock(header_info.fileDesc, 0, &block);

  int heap;
  int i;
  for(i=0;i<header_info.numBuckets; i++){
    if(i==h){
        memcpy(&heap, (char *)block + startup + sizeof(int)*i, sizeof(int));
        break;
    }
  }

  return HT_HP_GetAllEntries(&header_info, value, heap);
}

int HT_function(int* value, int buckets){
  char temp[32];
  sprintf(temp, "%d", *value);
  unsigned int hash = 5381; // djb2 hash function from data structures class
  for(int i=0;i<32;i++){
    hash = (hash << 5) + hash + temp[i];
  }
  return hash % buckets;
}

int HT_function(char* value, int buckets){
  unsigned int hash = 5381;
  for(char* s= value; *s != '\0'; s++){
    hash = (hash << 5) + hash + *s;
  }
  return hash % buckets;
}

#include "HT.h"

#include "BF.h"
#include <cstring>
#include <iostream>

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
  int h = record.id % header_info.numBuckets;

  int startup = 3+ sizeof(char) + MAX_ATTR_NAME_SIZE + sizeof(int) + sizeof(int);
  void* block;
  BF_ReadBlock(header_info.fileDesc, 0, &block);

  char* ht = new char[3];
  memcpy(ht, (char *)block, 3);
  if(strcmp(ht, "HT") != 0){
    printf("Error block\n");
    return -1;
  }
  block = (char*)block + startup;
  int heap;
  for(int i=0; i<header_info.numBuckets; i++){
    if(i==h){
      memcpy(&heap, (char*)block, sizeof(int));
      break;
    }else{
      block = (char *)block + sizeof(int);
    }
  }
  // heap_instert_to_block(heap, record) //opoy heap einai to heap id(oti epistrefei to BF_OpenFile sthn HP_Openfile->fileDesc)
  BF_WriteBlock(header_info.fileDesc, 0);
  return 0;
}

int HT_DeleteEntry(HT_info header_info, void *value){
  return 0;
}

int HT_GetAllEntries( HT_info header_info, void *value){
  return 0;
}

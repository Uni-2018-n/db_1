#include "HT.h"
#ifndef BF_H_
#define BF_H_
#include "BF.h"
#endif

#include <cstring>
#include <iostream>

int HT_CreateIndex(char *fileName, char attrType, char* attrName, int attrLength,int buckets){
  return 0;
}

HT_info* HT_OpenIndex(char *fileName){
  return 0;
}

int HT_CloseIndex(HT_info* header_info){
  return 0;
}

int HT_InsertEntry(HT_info header_info, Record record){
  return 0;
}

int HT_DeleteEntry(HT_info header_info, void *value){
  return 0;
}

int HT_GetAllEntries( HT_info header_info, void *value){
  return 0;
}

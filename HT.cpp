#include "HT.h"
#include "BF.h"
#include "HP.h"
#include <cstring>
#include <iostream>
#define MAX_BUCKETS_IN_BLOCK ((BLOCK_SIZE - 2 * (int) sizeof(int)) / (int) sizeof(int))

int HT_CreateIndex(const char *fileName, const char attrType, const char* attrName, const int attrLength, const int buckets){
  if(strlen(attrName) > MAX_ATTR_NAME_SIZE-1){ //failsafe if name is too big
    return -1;
  }
  BF_Init();

  if(BF_CreateFile(fileName) < 0){
    return -1;
  }

  int file = BF_OpenFile(fileName); //int file has the address of our file
  if(file < 0){
    return -1;
  }

  if(BF_AllocateBlock(file) <0){
    return -1;
  }

  void *block;
  if(BF_ReadBlock(file, 0, &block) <0){ //read the first block
    return -1;
  }
  //alocate space and save every data needed into the block
  memcpy((char *)block, "HT", 3); //safety to know that we have the corect file
	memcpy((char *)block + 3, &attrType, sizeof(char));
	memcpy((char *)block + 3 + sizeof(char), attrName, strlen(attrName) + 1);
	memcpy((char *)block + 3 + sizeof(char) + MAX_ATTR_NAME_SIZE , &attrLength, sizeof(int));
  memcpy((char *)block + 3 + sizeof(char) + MAX_ATTR_NAME_SIZE + sizeof(int), &buckets, sizeof(int));


  for(int i=0;i<buckets;i++){//after that, for every bucket allocate space to be able to save the address of the bucket. for now set as 0
    int temp =0;
    memcpy((char *)block + 3 + sizeof(char) + MAX_ATTR_NAME_SIZE + sizeof(int) + sizeof(int) + sizeof(int)*i, &temp, sizeof(int));
  }

  BF_WriteBlock(file, 0); //update the block so changes will be saved

  if(BF_CloseFile(file) < 0){
    return -1;
  }

  return 0;
}

HT_info* HT_OpenIndex(char *fileName){
  int file = BF_OpenFile(fileName); //open the file and read the data saved into the block and add it into the temp HT_info variable.
  if(file <0){
    return NULL;
  }
  HT_info* temp = new HT_info;
  temp->fileDesc = file;

  void* block;
  if(BF_ReadBlock(file, 0, &block)<0){
    return NULL;
  }
  char* ht = new char[3];
  memcpy(ht, (char *)block, 3);
  if(strcmp(ht, "HT") != 0){
    return NULL;
  }
  memcpy(&(temp->attrType), (char *)block+3, sizeof(char));
  memcpy(&(temp->attrName), (char *)block + 3+1, MAX_ATTR_NAME_SIZE);
  memcpy(&(temp->attrLength), (char *)block + 3+1 + MAX_ATTR_NAME_SIZE, sizeof(int));
  memcpy(&(temp->numBuckets), (char *)block + 3 + 1 + MAX_ATTR_NAME_SIZE + sizeof(int), sizeof(int));

  return temp;
}

int HT_CloseIndex(HT_info* header_info){
  int temp =BF_CloseFile(header_info->fileDesc);
  delete header_info;
  return temp;
}

int HT_InsertEntry(HT_info header_info, Record record){
  int h = HT_function(&record.id, header_info.numBuckets);//get the hash function output for that id and return a number from 0 to numBuckets
  int startup = 3+ sizeof(char) + MAX_ATTR_NAME_SIZE + sizeof(int) + sizeof(int);//we dont care to view anything of the "header" part of the block so skip that
  void* block;
  if(BF_ReadBlock(header_info.fileDesc, 0, &block) <0){
    return -1;
  }

  int heap;
  int i;
  for(i=0;i<header_info.numBuckets; i++){//for every bucket check if its the desired bucket ( if its the same as the hash function returned)
    if(i==h){
        memcpy(&heap, (char *)block + startup + sizeof(int)*i, sizeof(int)); //if it is save the heap's address
        break; //and stop the loop
    }
  }

  int new_heap_addr = HT_HP_InsertEntry(&header_info, &record, heap); //then pass it to HT_HP_InsertEntry to add it into the heap's blocks
  if (new_heap_addr == -1){
    return -1;
  }

  // printf("new_heap_addr: %d\n", new_heap_addr);
  if (new_heap_addr != heap){ //if the heap was empty the upper loop returned the int heap variable as 0 so we need to set the new address returned by HT_HP_InsertEntry
    memcpy((char*)block+startup+sizeof(int)*i, &new_heap_addr, sizeof(int));
    BF_WriteBlock(header_info.fileDesc, 0); //and save changed
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


  int startup = 3+ sizeof(char) + MAX_ATTR_NAME_SIZE + sizeof(int) + sizeof(int); //same thing as above we dont really care about the "header" data of the block
  void* block;
  if(BF_ReadBlock(header_info.fileDesc, 0, &block) <0){
    return -1;
  }

  int heap;
  int i;
  for(i=0;i<header_info.numBuckets; i++){//find the desired bucket and save the heap address
    if(i==h){
        memcpy(&heap, (char *)block + startup + sizeof(int)*i, sizeof(int));
        break;
    }
  }

  if (HT_HP_DeleteEntry(&header_info, value, heap) != 0){ // call the HT_HP_DeleteEntry to remove the entry from the heap
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

  int startup = 3+ sizeof(char) + MAX_ATTR_NAME_SIZE + sizeof(int) + sizeof(int);//same thing as above
  void* block;
  if(BF_ReadBlock(header_info.fileDesc, 0, &block) < 0){
    return -1;
  }

  int heap;
  int i;
  for(i=0;i<header_info.numBuckets; i++){//find out the bucket that the entry is in and return the heap's address
    if(i==h){
        memcpy(&heap, (char *)block + startup + sizeof(int)*i, sizeof(int));
        break;
    }
    memcpy(&heap, (char *)block + startup + sizeof(int)*i, sizeof(int));
  }

  return HT_HP_GetAllEntries(&header_info, value, heap); //call the function to print the entry and return the blocks searched to find the entry
}

int HT_function(int* value, int buckets){
  char temp[32];
  sprintf(temp, "%d", *value);
  unsigned int hash = 5381; // djb2 hash function from data structures class
  for(int i=0;temp[i] != '\0';i++){
    hash = (hash << 5) + hash + temp[i];
  }
  return hash % buckets;
}

int HT_function(char* value, int buckets){//same as int but for characters
  unsigned int hash = 5381;
  for(char* s= value; *s != '\0'; s++){
    hash = (hash << 5) + hash + *s;
  }
  return hash % buckets;
}



int HashStatistics(char* filename){
  HT_info* header_info = HT_OpenIndex(filename);

  void* block;
  if(BF_ReadBlock(header_info->fileDesc, 0, &block)<0){
    return -1;
  }

  int block_counter=1;

  int numBuckets =header_info->numBuckets;

  int startup = 3+ sizeof(char) + MAX_ATTR_NAME_SIZE + sizeof(int) + sizeof(int);

  int heap;

  memcpy(&heap, (char*)block + startup + sizeof(int)*0, sizeof(int));
  int array[numBuckets];
  for(int i=0;i<numBuckets;i++){
    memcpy(&array[i], (char*)block + startup + sizeof(int)*i, sizeof(int));
    printf("t: %d\n", array[i]);
  }
  int temp = HT_HP_GetRecordCounter(header_info, heap);
  int min = temp;
  int max = temp;
  int average_records = 0;
  int average_blocks = 0;
  printf("\n\n\n");
  for(int i=0;i< numBuckets; i++){
    heap = array[i];
    int num = HT_HP_GetBlockCounter(header_info, heap);

    int pl_records = HT_HP_GetRecordCounter(header_info, heap);
    block_counter += num;

    if(pl_records < min){
      min = pl_records;
    }
    if(pl_records > max){
      max = pl_records;
    }
    average_blocks += num;
    average_records += pl_records;
  }
  average_blocks = average_blocks/numBuckets;
  average_records = average_records/numBuckets;

  printf("Blocks used by file \"%s\": %d\n", filename, block_counter);
  printf("Minimum records per bucket: %d\nMaximum records per bucket: %d\nAverage: %d\n", min, max, average_records);
  printf("Average number of blocks per bucket: %d\n", average_blocks);

  printf("Overflow blocks: \n");
  int overflow=0;
  for(int i=0;i<numBuckets;i++){
    int heap;
    heap = array[i];
    int num = HT_HP_GetBlockCounter(header_info, heap);
    if(num > 1){
      overflow++;
      printf("For bucket %d, %d\n", i, num-1);
    }
  }
  printf("Overflow sum: %d\n", overflow);

  if(HT_CloseIndex(header_info) <0){
    return -1;
  }
  return 0;
}

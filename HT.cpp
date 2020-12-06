#include "HT.h"
#include "BF.h"
#include "HP.h"
#include <cstring>
#include <iostream>
#define MAX_BUCKETS_IN_BLOCK ((BLOCK_SIZE - 2 * (int) sizeof(int)) / (int) sizeof(int))

using namespace std;
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

  int pl_blocks = (buckets / MAX_BUCKETS_IN_BLOCK)+ 1;
  for(int i=0;i<pl_blocks;i++){
    if(BF_AllocateBlock(file) < 0){
      return -1;
    }
    if(BF_ReadBlock(file, BF_GetBlockCounter(file)-1, &block) < 0){
      return -1;
    }
    int max;
    if(i == pl_blocks-1){
      max = buckets - MAX_BUCKETS_IN_BLOCK*i;
    }else{
      max = MAX_BUCKETS_IN_BLOCK;
    }

    for(int j=0;j<max;j++){
      int temp =0;
      memcpy((char*)block+ sizeof(int)*j, &temp, sizeof(int));
    }
    BF_WriteBlock(file, BF_GetBlockCounter(file)-1);
  }

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
  // int startup = 3+ sizeof(char) + MAX_ATTR_NAME_SIZE + sizeof(int) + sizeof(int);//we dont care to view anything of the "header" part of the block so skip that
  void* block;
  int heap;
  int j;
  int i;
  int counter=0;
  int pl_blocks = (header_info.numBuckets / MAX_BUCKETS_IN_BLOCK)+ 1;

  for(i=0;i<pl_blocks;i++){
    if(BF_ReadBlock(header_info.fileDesc, i+1, &block) <0){
      return -1;
    }

    int max;
    heap=0;
    if(i == pl_blocks-1){
      max = header_info.numBuckets - MAX_BUCKETS_IN_BLOCK*i;
    }else{
      max = MAX_BUCKETS_IN_BLOCK;
    }
    int found =0;
    for(j=0;j<max;j++){
      if(counter == h){
        memcpy(&heap, (char *)block + sizeof(int)*j, sizeof(int)); //if it is save the heap's address
        found =1;
        break;
      }
      counter++;
    }
    if(found){
      break;
    }

  }
  int new_heap_addr = HT_HP_InsertEntry(&header_info, &record, heap); //then pass it to HT_HP_InsertEntry to add it into the heap's blocks
  if (new_heap_addr == -1){
    return -1;
  }

  // printf("new_heap_addr: %d\n", new_heap_addr);
  if (new_heap_addr != heap){ //if the heap was empty the upper loop returned the int heap variable as 0 so we need to set the new address returned by HT_HP_InsertEntry
    memcpy((char*)block +sizeof(int)*j, &new_heap_addr, sizeof(int));
    BF_WriteBlock(header_info.fileDesc, i+1); //and save changed
  }


  // this tests if hash_table's addresses is working (check if something changes from 0)
  // for(int i=0;i<pl_blocks;i++){
  //   printf("for block: %d\n", i+1);
  //   if(BF_ReadBlock(header_info.fileDesc, i+1, &block)< 0){
  //     return -1;
  //   }
  //   int max;
  //   if(i == pl_blocks-1){
  //     max = header_info.numBuckets - MAX_BUCKETS_IN_BLOCK*i;
  //   }else{
  //     max = MAX_BUCKETS_IN_BLOCK;
  //   }
  //   int heap;
  //   for(int j=0;j<max;j++){
  //     memcpy(&heap, (char*)block + sizeof(int)*j, sizeof(int));
  //     printf("hash table: %d\n", heap);
  //   }
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


  void* block;
  int heap;
  int j;
  int i;
  int counter=0;
  int pl_blocks = (header_info.numBuckets / MAX_BUCKETS_IN_BLOCK)+ 1;

  for(i=0;i<pl_blocks;i++){
    if(BF_ReadBlock(header_info.fileDesc, i+1, &block) <0){
      return -1;
    }

    int max;
    heap=0;
    if(i == pl_blocks-1){
      max = header_info.numBuckets - MAX_BUCKETS_IN_BLOCK*i;
    }else{
      max = MAX_BUCKETS_IN_BLOCK;
    }
    int found =0;
    for(j=0;j<max;j++){
      if(counter == h){
        memcpy(&heap, (char *)block + sizeof(int)*j, sizeof(int)); //if it is save the heap's address
        found =1;
        break;
      }
      counter++;
    }
    if(found){
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

  void* block;
  int heap;
  int j;
  int i;
  int counter=0;
  int pl_blocks = (header_info.numBuckets / MAX_BUCKETS_IN_BLOCK)+ 1;

  for(i=0;i<pl_blocks;i++){
    if(BF_ReadBlock(header_info.fileDesc, i+1, &block) <0){
      return -1;
    }

    int max;
    heap=0;
    if(i == pl_blocks-1){
      max = header_info.numBuckets - MAX_BUCKETS_IN_BLOCK*i;
    }else{
      max = MAX_BUCKETS_IN_BLOCK;
    }
    int found =0;
    for(j=0;j<max;j++){
      if(counter == h){
        memcpy(&heap, (char *)block + sizeof(int)*j, sizeof(int)); //if it is save the heap's address
        found =1;
        break;
      }
      counter++;
    }
    if(found){
      break;
    }

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

  int numBuckets =header_info->numBuckets;
  int pl_blocks = (numBuckets / MAX_BUCKETS_IN_BLOCK)+ 1;
  int block_counter=1 + pl_blocks;

  int array[numBuckets];

  void* block;
  int heap;
  int j;
  int i;
  int counter=0;

  for(i=0;i<pl_blocks;i++){
    if(BF_ReadBlock(header_info->fileDesc, i+1, &block) <0){
      return -1;
    }

    int max;
    heap=0;
    if(i == pl_blocks-1){
      max = header_info->numBuckets - MAX_BUCKETS_IN_BLOCK*i;
    }else{
      max = MAX_BUCKETS_IN_BLOCK;
    }
    for(j=0;j<max;j++){
        memcpy(&heap, (char *)block + sizeof(int)*j, sizeof(int));
        array[counter] = heap;
      counter++;
    }
  }
  int temp = HT_HP_GetRecordCounter(header_info, array[0]);
  int min = temp;
  int max = temp;
  int average_records = 0;
  int average_blocks = 0;
  for(int i=0;i< numBuckets; i++){
    heap = array[i];
    int num = HT_HP_GetBlockCounter(header_info, heap);//here it goes into infinite loop

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

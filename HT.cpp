#include "HT.h"

#include "BF.h"
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

  // No reason to check if it is HT or not.
  // You already did it when you opened the file.
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

  // heap_instert_to_block(heap, record) //TODO: opoy heap einai to heap id(oti epistrefei to BF_OpenFile sthn HP_Openfile->fileDesc)
  // I assume that heap equals 0 if there isn't a heap yet and the address of the heap if the is.
  // I return the address of the first element. 
  // If it is different than then one you had before, you should write the block.
  // No need to write if you don't need to.

  // Uncomment this. And delete the BF_Write_Block at the end.
  // int new_heap_addr = HT_HP_InsertEntry(&header_info, &record, heap);
  // if (new_heap_addr == -1)
  //   return -1;
  
  // if (new_heap_addr != heap)
  //   BF_WriteBlock(header_info.fileDesc, 0);

  BF_WriteBlock(header_info.fileDesc, 0);

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
  char* ht = new char[3];
  memcpy(ht, (char *)block, 3);
  if(strcmp(ht, "HT") != 0){
    printf("Error block\n");
    return -1;
  }
  block = (char*)block +startup;
  int heap;
  for(int i=0;i<header_info.numBuckets;i++){
    if(i==h){
      memcpy(&heap, (char *)block, sizeof(int));
      break;
    }else{
      block = (char *)block + sizeof(int);
    }
  }
  //heap_delete_entry(heap_number, id) //TODO: mporw na dosw to heap number kai mono to id oxi olo to record...
  // I assume again that heap equals 0 if there isn't a heap yet and the address of the heap if the is.
  // I return 0 if the deletion was successful and -1 if the key wasn't found or there was an error.
  // Uncomment this to use it.
  // if (HT_HP_DeleteEntry(&header_info, value, heap) != 0)
  //   return -1;

  return 0;
}

int HT_GetAllEntries( HT_info header_info, void *value){
  // Do stuff.

  // I assume again that heap equals 0 if there isn't a heap yet and the address of the heap if the is.
  // I return the number of blocks that we had to read if successfull, -1 if not.
  // Uncomment this to use it. You can remove the last return 0; You can remove the last return 0.
  // return HT_HP_GetAllEntries(&header_info, value, heap);
  return 0;
}

int HT_function(int* value, int buckets){
  return *value % buckets;
}

int HT_function(char* value, int buckets){
  //TODO: make this
  return 0;
}

int HT_HP_GetAllEntries(HT_info* header_info, void* value, int heap_addr)
{
  if (heap_addr == 0)
    return -1;

	void* block;
	int curr_block_addr = heap_addr;
	Record record;
  int counter = 0; // Counts how many blocks were read until we found the key.

  while (curr_block_addr != -1)
	{
    counter++;

		if (BF_ReadBlock(header_info->fileDesc, curr_block_addr, &block) != 0)
			return -1;

		if (AssignKeyToRecord(&record, value, header_info->attrType) != 0)
			return 1;

		int record_pos = IsKeyInBlock(&record, block);
		if (record_pos > -1)
		{
			ReadRecord(block, record_pos, &record);

			std::cout << "id: " << record.id
					  << "\nname: " << record.name
					  << "\nsurname: " << record.surname
					  << "\naddress: " << record.address
					  << std::endl;

			return counter;
		}

    curr_block_addr = ReadNextBlockAddr(block);
	}

	return -1;
}

int HT_HP_DeleteEntry(HT_info* header_info, void* value, int heap_address)
{
  if (heap_address == 0)
    return -1;

  void* block;
	Record record;
	if (AssignKeyToRecord(&record, value, header_info->attrType) != 0)
		return 1;

  int curr_block_addr = heap_address;

  while (curr_block_addr != -1)
	{
		if (BF_ReadBlock(header_info->fileDesc, curr_block_addr, &block) != 0)
			return -1;

		int pos = IsKeyInBlock(&record, block);
		if (pos > -1)
		{
			ReplaceWithLastRecord(pos, block);

			// Lower num_of_records.
			WriteNumOfRecords(block, ReadNumOfRecords(block) - 1);

			BF_WriteBlock(header_info->fileDesc, curr_block_addr);

			return 0;
		}

    curr_block_addr = ReadNextBlockAddr(block);
	}

	// Key wasn't found.
	return -1;
}

int HT_HP_InsertEntry(HT_info* header_info, Record* record, int heap_address)
{
	int curr_block_addr = heap_address;
  int should_init_block = 0;

  // First block in heap.
  if (heap_address == 0)
  {
    should_init_block = 1;
    // Set the address that the heap is going to get.
    heap_address = BF_GetBlockCounter(header_info->fileDesc);
  }

  void* block = nullptr;

	int available_block_addr = -1; // a block with space to add a record.
  int next_block_addr = -1;

	// Loop until you have read all the blocks.
	while(1)
	{
		if (should_init_block == 0)
		{
			if (BF_ReadBlock(header_info->fileDesc, curr_block_addr, &block) != 0)
				return 1;
		}

		else
		{
			// Create a block and initialize some values.
			if (InitBlock(header_info->fileDesc, &block) == -1)
				return -1;
      
			should_init_block = 0;
		}

		if (IsKeyInBlock(record, block) > -1)
			return -1;

		int num_of_records = ReadNumOfRecords(block);

		// If we haven't enough space for another record.
		if (num_of_records + 1 > MAX_RECORDS_IN_BLOCK)
		{
			next_block_addr = ReadNextBlockAddr(block);

			// If there isn't a next block and we haven't found an available address for a record.
			if (next_block_addr == -1 && available_block_addr == -1)
			{
        int num_of_blocks = BF_GetBlockCounter(header_info->fileDesc);
				WriteNextBlockAddr(block, num_of_blocks);
				BF_WriteBlock(header_info->fileDesc, curr_block_addr);
        next_block_addr = num_of_blocks;

				should_init_block = 1;
			}
		}

		// If we have space to store the block and haven't assigned one yet.
		else if (available_block_addr == -1)
			available_block_addr = curr_block_addr;

		// If we have reached the last block.
		if (next_block_addr == -1)
		{
			// Only read when necessary.
			if (curr_block_addr != available_block_addr)
			{
				if (BF_ReadBlock(header_info->fileDesc, available_block_addr, &block) != 0)
					return -1;
			}

			WriteRecord(block, num_of_records, record);
			WriteNumOfRecords(block, num_of_records + 1);

			if (BF_WriteBlock(header_info->fileDesc, available_block_addr) != 0)
				return -1;

			break;
		}

    curr_block_addr = next_block_addr;
	}

	return heap_address;
}

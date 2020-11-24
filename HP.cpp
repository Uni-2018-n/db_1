#include "HP.h"
#include "BF.h"

#include <cstring>
#include <iostream>

const int MAX_RECORDS_IN_BLOCK = (BLOCK_SIZE - sizeof(HP_info)) / sizeof(Record);

int HP_CreateFile(const char *fileName, char attrType, const char* attrName, int attrLength)
{
	// Don't do anything if name bigger than the limit.
	if (strlen(attrName) > MAX_ATTR_NAME_SIZE - 1)
		return 1;

	BF_Init();

	int error = BF_CreateFile(fileName);
	if (error != 0)
		return error;

	int fd = BF_OpenFile(fileName);
	if (fd < 0) 
		return fd;

	error = BF_AllocateBlock(fd);
	if (error != 0)
			return error;

	void *block;
	error = BF_ReadBlock(fd, 0, &block);
	if (error != 0)
		return error;

	// Copy data to block.
	memcpy((char *)block, "HP", 3);
	memcpy((char *)block + 3, &attrType, sizeof(char));
	memcpy((char *)block + 3 + sizeof(char), attrName, strlen(attrName) + 1);
	memcpy((char *)block + 3 + sizeof(char) + MAX_ATTR_NAME_SIZE , &attrLength, sizeof(int));	

	error = BF_WriteBlock(fd, 0);
		if (error != 0)
			return error;

	error = BF_CloseFile(fd);
	if (error != 0)
		return error;

	return 0;
}

HP_info* HP_OpenFile(const char *fileName)
{
	BF_Init();

	int fd = BF_OpenFile(fileName);
	if (fd < 0)
		return nullptr;

	HP_info* header_info = new HP_info;

	header_info->fileDesc = fd;

	void *block;
	int error = BF_ReadBlock(fd, 0, &block);
	if (error != 0)
		return nullptr;

	memcpy(&(header_info->type), (char *)block, 3);
	// Check if the file type is compatible.
	if (strcmp(header_info->type, "HP") != 0)
		return nullptr;

	memcpy(&(header_info->attrType), (char *)block + 3, sizeof(char));
	memcpy(header_info->attrName, ((char *)block) + 3 + sizeof(char), MAX_ATTR_NAME_SIZE);
	memcpy(&(header_info->attrLength) , (char *)block + 3 + sizeof(char) + MAX_ATTR_NAME_SIZE, sizeof(int));	

	return header_info;
}

int HP_CloseFile(HP_info* header_info)
{
	int error = BF_CloseFile(header_info->fileDesc);
	if (error != 0)
		return error;

	delete header_info;

	return 0;
}

int ReadNumOfRecords(void* block)
{
	int num_of_records;

	memcpy(&num_of_records, (char *)block + BLOCK_SIZE - sizeof(int) * 2, sizeof(int));
	
	return num_of_records;
}

void WriteNumOfRecords(void* block, int recordNumber)
{
	memcpy((char *)block + BLOCK_SIZE - sizeof(int) * 2, &recordNumber, sizeof(int)); 
}

int ReadNextBlockAddr(void* block)
{
	int next_block_addr;

	memcpy(&next_block_addr, (char *)block + BLOCK_SIZE - sizeof(int), sizeof(int));

	return next_block_addr;
}

void WriteNextBlockAddr(void* block, int blockAddrNumber)
{
	memcpy((char *)block + BLOCK_SIZE - sizeof(int), &blockAddrNumber, sizeof(int)); 
}

void ReadRecord(void* block, int recordNumber, Record* record)
{
	memcpy(record, (char *)block + recordNumber * sizeof(Record), sizeof(Record));	
}

void WriteRecord(void* block, int recordNumber, const Record* record)
{
	memcpy((char *)block + recordNumber * sizeof(Record), record, sizeof(Record));	
}

// !This might need void**
void* InitBlock(HP_info* header_info, int blockNumber)
{
	void* block = nullptr;

	int error = BF_AllocateBlock(header_info->fileDesc);
	if (error != 0)
		return nullptr;

	BF_ReadBlock(header_info->fileDesc, blockNumber, &block);
	if (error != 0)
		return nullptr;

	WriteNumOfRecords(block, 0);
	WriteNextBlockAddr(block, -1);

	return block;
}

int IsKeyInBlock(Record* record, void* block)
{
	int num_of_records = ReadNumOfRecords(block);

	Record tmp_record;

	for (int i = 0; i < num_of_records; i++)
	{
		ReadRecord(block, i, &tmp_record);

		if (record->id == tmp_record.id)
			return i;
	}

	return -1;
}

int HP_InsertEntry(HP_info header_info, Record record)
{
	int num_of_blocks = BF_GetBlockCounter(header_info.fileDesc);

	void* block = nullptr;

	int curr_block_addr = 1; // 1 cause we don't need to read the header.
	int available_block_addr = -1; // a block with space to add a record.

	// Only the header.
	if (num_of_blocks == 1)
	{
		block = InitBlock(&header_info, 1);
		if (block == nullptr)
			return -1;

		// We just created the block, so we don't need to read it, again.
		goto skip_reading_file_cause_it_is_the_first;
	}
		
	while (true)
	{
		if (BF_ReadBlock(header_info.fileDesc, curr_block_addr, &block) != 0)
			return -1;

		skip_reading_file_cause_it_is_the_first:

		if (IsKeyInBlock(&record, block) == -1)
			return -1;

		int num_of_records = ReadNumOfRecords(block);
		// If we haven't space for a next block.
		if (num_of_records + 1 >= MAX_RECORDS_IN_BLOCK)
		{
			curr_block_addr++;
			int next_block_addr = ReadNextBlockAddr(block);
			// There isn't a next block.
			if (next_block_addr == -1)
			{
				available_block_addr = curr_block_addr;
				block = InitBlock(&header_info, curr_block_addr);

				// We already know this, no need to read it.
				num_of_records = 0;

				// We just created the block, so we don't need to read it, again.
				goto skip_reading_block_cause_it_is_new;
			}

			// We don't have space and there's a next block, so we continue.
			else 
				continue;
		}

		else if (available_block_addr != -1)
		{
			available_block_addr = curr_block_addr;
		}

		// If we have reached the last block and we haven't found the key.
		if (curr_block_addr == num_of_blocks)
		{
			if (BF_ReadBlock(header_info.fileDesc, available_block_addr, &block) != 0)
				return -1;

			skip_reading_block_cause_it_is_new:

			WriteRecord(block, num_of_records, &record);
			WriteNumOfRecords(block, ++num_of_records);
			break;
		}
	}

	return available_block_addr;
}

// !Maybe I should use (int)*value.
int AssignKeyToRecord(Record* record, void* value, char key_type)
{
	switch (key_type)
	{
	case 'i':
		record->id = *(int *)value;
		return 0;
	case 'c':
		record->id = *(char *)value;
		return 0;
	default:
		return 1;
	}
}

int IsBlockEmpty(int file_desc)
{
	return BF_GetBlockCounter(file_desc) > 1;
}

void ReplaceWithLastRecord(int pos, void* block)
{
	int num_of_records = ReadNumOfRecords(block);	

	// Last record, no need to do anything.
	if (num_of_records == pos - 1)
		return;

	Record record;
	ReadRecord(block, num_of_records - 1, &record);
	WriteRecord(block, pos, &record);
}

int HP_DeleteEntry(HP_info header_info, void *value)
{
	if (IsBlockEmpty(header_info.fileDesc))
		return -1;

	void* block;
	int curr_block_addr;
	Record record;

	for (;;curr_block_addr++)
	{
		if (BF_ReadBlock(header_info.fileDesc, curr_block_addr, &block) != 0)
			return -1;

		if (AssignKeyToRecord(&record, value, header_info.attrType) != 0)
			return 1;

		int pos = IsKeyInBlock(&record, block);
		if (pos >= -1)
		{
			ReplaceWithLastRecord(pos, block);

			// Lower num_of_records.
			WriteNumOfRecords(block, ReadNumOfRecords(block) - 1);
			
			return 0;
		}
	}

	// Key wasn't found.
	return -1;
}

int HP_GetAllEntries(HP_info header_info, void* value)
{
	if (IsBlockEmpty(header_info.fileDesc))
		return -1;

	void* block;
	int curr_block_addr;
	Record record;

	for (;;curr_block_addr++)
	{
		if (BF_ReadBlock(header_info.fileDesc, curr_block_addr, &block) != 0)
			return -1;

		if (AssignKeyToRecord(&record, value, header_info.attrType) != 0)
			return 1;

		if (IsKeyInBlock(&record, block) >= -1)
		{
			std::cout << "id: " << record.id
					  << "\nname: " << record.name
					  << "\nsurname: " << record.surname
					  << "\naddress: " << record.address
					  << std::endl;

			return curr_block_addr;
		}
	}

	// Key wasn't found.
	return -1;
}

// TODO: Remove this function.
void PrintAllEntries(HP_info header_info)
{
	if (IsBlockEmpty(header_info.fileDesc))
		return;

	void* block;
	Record record;

	int num_of_blocks = BF_GetBlockCounter(header_info.fileDesc);

	for (int i = 0; i < num_of_blocks; i++)
	{
		if (BF_ReadBlock(header_info.fileDesc, i, &block) != 0)
			return;

		int num_of_records = ReadNumOfRecords(block);

		for (int y = 0; y < num_of_records; y++)
		{
			ReadRecord(block, y, &record);

			std::cout << "block: " << i
					  << "\nid: " << record.id
					  << "\nname: " << record.name
					  << "\nsurname: " << record.surname
					  << "\naddress: " << record.address
					  << std::endl << std::endl;
					  // TODO: remove second std::endl.
		}
	}
}

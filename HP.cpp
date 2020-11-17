#include "HP.h"
#include "BF.h"

#include <cstring>

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

	HP_info* info = new HP_info;

	info->fileDesc = fd;

	void *block;
	int error = BF_ReadBlock(fd, 0, &block);
	if (error != 0)
		return nullptr;

	memcpy(&(info->type), (char *)block, 3);
	// Check if the file type is compatible.
	if (strcmp(info->type, "HP") != 0)
		return nullptr;

	memcpy(&(info->attrType), (char *)block + 3, sizeof(char));
	memcpy(info->attrName, ((char *)block) + 3 + sizeof(char), MAX_ATTR_NAME_SIZE);
	memcpy(&(info->attrLength) , (char *)block + 3 + sizeof(char) + MAX_ATTR_NAME_SIZE, sizeof(int));	

	return info;
}

int HP_CloseFile(HP_info* header_info)
{
	int error = BF_CloseFile(header_info->fileDesc);
	if (error != 0)
		return error;

	delete header_info;

	return 0;
}

// int HP_InsertEntry(HP_info header_info, Record record)
// {
// 	return 0;
// }

// int HP_DeleteEntry(HP_info header_info, void *value)
// {
// 	return 0;
// }

// int HP_GetAllEntries(HP_info header_info, void *value)
// {
// 	return 0;
// }

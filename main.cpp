#include <iostream>

#include "HT.h"
#include <cstring>
int main(){
	char temp[256] = "temp";
	HT_CreateIndex(temp, 'i', "id", 14, 8);

	HT_info* t = HT_OpenIndex(temp);
	printf("t: %d %c %s %d %ld\n", t->fileDesc, t->attrType, t->attrName, t->attrLength, t->numBuckets);

	Record yeet;
	yeet.id = 3;
	strcpy(yeet.name, "name");
	strcpy(yeet.name, "surname");
	strcpy(yeet.address, "address");

	printf("HT_InsertEntry: %d\n\n", HT_InsertEntry(*t, yeet));
	printf("\nHT_GetAllEntries(gives the number of blocks needed to read): %d\n", HT_GetAllEntries(*t, &(yeet.id)));
	printf("HT_DeleteEntry: %d\n", HT_DeleteEntry(*t, &(yeet.id)));


	printf("close index: %d\n", HT_CloseIndex(t));
	return 0;
}
